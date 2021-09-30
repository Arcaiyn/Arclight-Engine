#pragma once

#include "pixel.h"
#include "types.h"
#include "math/math.h"
#include "math/vector.h"
#include "util/assert.h"
#include <vector>
#include <span>
#include <array>



enum class ImageScaling {
    Nearest,
    Bilinear
};


template<Pixel P = Pixel::RGB8>
class Image {

public:

    using Format = PixelFormat<P>;
    using PixelType = PixelType<P>::Type;

    constexpr static u32 PixelBytes = Format::BytesPerPixel;


    constexpr Image() : Image(0, 0) {}
    constexpr Image(u32 width, u32 height) : width(width), height(height) {
        data.resize(width * height);
    }

    constexpr void setRawData(const std::span<u8>& src, u64 startPixel = 0) {

        SizeT pixels = data.size() / PixelBytes;
        arc_assert(startPixel + pixels <= data.size(), "Cannot copy pixel data to smaller image");

        for(SizeT i = 0; i < pixels; i++) {
            data[i + startPixel] = Format::fromBytes(data.subspan(src * PixelBytes));
        }

    }

    template<Pixel Q>
    constexpr Image<Q> convertTo() const {

        Image<Q> image(width, height);

        for(u64 i = 0; i < width * height; i++) {

            auto pixel = Format::inflate(data[i]);
            image.data[i] = PixelFormat<Q>::deflate(pixel);

        }

        return image;

    }

    constexpr u32 getWidth() const {
        return width;
    }

    constexpr u32 getHeight() const {
        return height;
    }

    constexpr std::span<PixelType> getImageBuffer() {
        return data;
    }

    constexpr std::span<const PixelType> getImageBuffer() const {
        return data;
    }

    constexpr void setPixel(u32 x, u32 y, const PixelType& pixel) {

        arc_assert(x < width && y < height, "Pixel access out of bounds");
        data[y * width + x] = pixel;

    }

    constexpr const PixelType& getPixel(u32 x, u32 y) const {
        
        arc_assert(x < width && y < height, "Pixel access out of bounds");
        return data[y * width + x];

    }

    constexpr PixelType& getPixel(u32 x, u32 y) {
        
        arc_assert(x < width && y < height, "Pixel access out of bounds");
        return data[y * width + x];

    }

    template<class Filter, class... Args>
    void applyFilter(Args&&... args) {
        Filter::run(*this, std::forward<Args>(args)...);
    }

    constexpr void resize(ImageScaling scaling, u32 w, u32 h = 0) {

        if(!w) {
            Log::error("Image", "Cannot resize image to a width of 0");
            return;
        }


        if(!h) {

            //Take the aspect ratio
            float aspect = static_cast<float>(width) / height;
            h = static_cast<u32>(Math::round(w / aspect));

        }

        //Skip case
        if(w == width && h == height) {
            return;
        }

        std::vector<PixelType> resizedData(w * h);

        switch(scaling) {

            case ImageScaling::Nearest:

                for(u32 y = 0; y < h; y++) {

                    u32 cy = static_cast<u32>(Math::floor((y + 0.5) * height / h));

                    for(u32 x = 0; x < w; x++) {

                        u32 cx = static_cast<u32>(Math::floor((x + 0.5) * width / w));
                        resizedData[y * w + x] = getPixel(cx, cy);

                    }

                }

                break;

            case ImageScaling::Bilinear:

                for(u32 y = 0; y < h; y++) {

                    float fy = (y + 0.5f) * height / h;
                    float ty = fy - static_cast<u32>(fy);

                    u32 cy0, cy1;

                    if(ty >= 0.5) {
                        cy0 = static_cast<u32>(fy);
                        cy1 = Math::min(cy0 + 1, height - 1);
                    } else {
                        cy1 = static_cast<u32>(fy);
                        cy0 = cy1 ? cy1 - 1 : 0;
                        ty += 1;
                    }
                    
                    float dy = ty - 0.5f;

                    for(u32 x = 0; x < w; x++) {

                        float fx = (x + 0.5f) * width / w;
                        float tx = fx - static_cast<u32>(fx);
                        u32 cx0, cx1;

                        if(tx >= 0.5) {
                            cx0 = static_cast<u32>(fx);
                            cx1 = Math::min(cx0 + 1, width - 1);
                        } else {
                            cx1 = static_cast<u32>(fx);
                            cx0 = cx1 ? cx1 - 1 : 0;
                            tx += 1;
                        }

                        float dx = tx - 0.5f;
                        
                        const PixelType& p00 = getPixel(cx0, cy0);
                        const PixelType& p01 = getPixel(cx0, cy1);
                        const PixelType& p10 = getPixel(cx1, cy0);
                        const PixelType& p11 = getPixel(cx1, cy1);

                        //No need to check for max pixel values since those are impossible to reach by standard interpolation
                        Vec4f v00(p00.getRed(), p00.getGreen(), p00.getBlue(), p00.getAlpha());
                        Vec4f v01(p01.getRed(), p01.getGreen(), p01.getBlue(), p01.getAlpha());
                        Vec4f v10(p10.getRed(), p10.getGreen(), p10.getBlue(), p10.getAlpha());
                        Vec4f v11(p11.getRed(), p11.getGreen(), p11.getBlue(), p11.getAlpha());
                        
                        Vec4f a0 = (1.0f - dx) * v00 + dx * v10;
                        Vec4f a1 = (1.0f - dx) * v01 + dx * v11;
                        Vec4f a = (1.0f - dy) * a0 + dy * a1;
                        
                        PixelType p;
#ifdef ARC_PIXEL_EXACT
                        p.setRGBA(static_cast<u32>(Math::round(a.x)), static_cast<u32>(Math::round(a.y)), static_cast<u32>(Math::round(a.z)), static_cast<u32>(Math::round(a.w)));
#else
                        p.setRGBA(static_cast<u32>(a.x), static_cast<u32>(a.y), static_cast<u32>(a.z), static_cast<u32>(a.w));
#endif
                        resizedData[y * w + x] = p;

                    }

                }

                break;

            default:
                arc_force_assert("Illegal scaling parameter");
                break;

        }
        
        width = w;
        height = h;
        data.swap(resizedData);

    }


private:

    u32 width;
    u32 height;
    std::vector<PixelType> data;

};