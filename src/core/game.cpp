#include "game.h"
#include "window.h"
#include "render/physicsrenderer.h"
#include "stdext/optionalref.h"
#include "stdext/sparsearray.h"
#include "stdext/any.h"
#include "debug.h"
#include "acs/actorblueprints.h"
#include "util/profiler.h"
#include "acs/componentview.h"
#include "acs/component/transform.h"
#include "acs/component/model.h"
#include "input/inputcontext.h"
#include "math/quaternion.h"
#include "util/file.h"
#include "stream/byteinputstream.h"
#include "stream/fileinputstream.h"
//#include "compress/lz77.h"


Game::Game(Window& window) : window(window), physicsEngine(manager), renderer(manager) {}

Game::~Game() {}


bool Game::init() {

	profiler.start();

	window.disableVSync();

	renderer.init();
	renderer.setAspectRatio(window.getWidth() / static_cast<float>(window.getHeight()));

	window.setFramebufferResizeFunction([this](u32 w, u32 h) { 
		GLE::Framebuffer::setViewport(w, h);
		renderer.setAspectRatio(window.getWidth() / static_cast<float>(window.getHeight())); 
	});

	if (!imageRenderer.init()) {
		return false;
	}
	

	window.setSize(720, 720);

	//Connect the input system to the window in order to receive events
	inputSystem.connect(window);

	//Create the root context and add actions to it
	InputContext& rootContext = inputSystem.createContext(0);
	rootContext.addState(0);

	rootContext.addAction(0, KeyTrigger({ KeyCode::W }), false);
	rootContext.addAction(1, KeyTrigger({ KeyCode::A }), false);
	rootContext.addAction(2, KeyTrigger({ KeyCode::S }), false);
	rootContext.addAction(3, KeyTrigger({ KeyCode::D }), false);
	rootContext.addAction(4, KeyTrigger({ KeyCode::Up }), false);
	rootContext.addAction(5, KeyTrigger({ KeyCode::Down }), false);
	rootContext.registerAction(0, 0);
	rootContext.registerAction(0, 1);
	rootContext.registerAction(0, 2);
	rootContext.registerAction(0, 3);
	rootContext.registerAction(0, 4);
	rootContext.registerAction(0, 5);

	//Define input handler callbacks
	rootInputHandler.setActionListener([this](KeyAction action) {
		
		switch(action) {

			case 0:
				imageRenderer.moveCanvas(ImageRenderer::KeyAction::Up);
				break;

			case 1:
				imageRenderer.moveCanvas(ImageRenderer::KeyAction::Left);
				break;

			case 2:
				imageRenderer.moveCanvas(ImageRenderer::KeyAction::Down);
				break;

			case 3:
				imageRenderer.moveCanvas(ImageRenderer::KeyAction::Right);
				break;

			case 4:
				imageRenderer.moveCanvas(ImageRenderer::KeyAction::ZoomIn);
				break;

			case 5:
				imageRenderer.moveCanvas(ImageRenderer::KeyAction::ZoomOut);
				break;

			default:
				return false;

		}

		return true;

	});


	//Link handler to the root context
	rootContext.linkHandler(rootInputHandler);

	profiler.stop("Initialization");

	inputTicker.start(120);

	return true;

}



void Game::update() {

	inputSystem.updateContinuous(inputTicker.getTicks());

}



void Game::render() {

	imageRenderer.render();

}



void Game::destroy() {

	profiler.start();
	renderer.destroy();
	imageRenderer.destroy();

	physicsEngine.destroy();

	profiler.stop("Destruction");

}