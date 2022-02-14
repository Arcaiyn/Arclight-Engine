/*
 *	 Copyright (c) 2022 - Arclight Team
 *
 *	 This file is part of Arclight. All rights reserved.
 *
 *	 spriterenderer.hpp
 */

#pragma once

#include "compositetexture.hpp"
#include "spritearray.hpp"
#include "spritebatch.hpp"
#include "spritegroup.hpp"
#include "spritefactory.hpp"
#include "spritetypebuffer.hpp"
#include "math/rectangle.hpp"

#include <map>
#include <memory>



class TextureSet;

class SpriteRenderer {

public:

	SpriteRenderer();

	void preload();
	void render();

	void setViewport(const Vec2f& lowerLeft, const Vec2f& topRight);

	//Sprite functions
	Sprite& createSprite(Id64 id, Id32 typeID, Id32 groupID = 0);
	Sprite& getSprite(Id64 id);
	const Sprite& getSprite(Id64 id) const;
	bool containsSprite(Id64 id) const;
	void destroySprite(Id64 id);

	//Type functions
	void createType(Id32 id, Id32 textureID, const Vec2f& size);
	void createType(Id32 id, Id32 textureID, const Vec2f& size, const Vec2f& origin, const SpriteOutline& outline = SpriteOutline());
	bool hasType(Id32 id) const;
	const SpriteType& getType(Id32 id) const;

	//Texture functions
	void loadTextureSet(const TextureSet& set);

	//Group functions
	void showGroup(Id32 groupID);
	void hideGroup(Id32 groupID);
	void setGroupVisibility(Id32 groupID, bool visible);
	void toggleGroupVisibility(Id32 groupID);
	bool isGroupVisible(Id32 groupID) const;

	u32 activeSpriteCount() const noexcept;

private:

	constexpr static u32 textureSlotRange = -1;

	Mat2f calculateSpriteTransform(const Sprite& sprite) const;
	Vec2f calculateGlobalSpriteScale(const Sprite& sprite) const;

	void recalculateProjection();

	SpriteFactory factory;
	SpriteArray sprites;

	std::shared_ptr<class SpriteRendererShaders> shaders;

	SpriteTypeBuffer typeBuffer;
	std::map<u32, SpriteGroup> groups;
	std::map<u32, SpriteBatch> batches;

	std::vector<CompositeTexture> textures;
	std::unordered_map<u64, u32> texComposites;

	RectF viewport;          //The scene's viewport rect
	Mat4f projection;

};