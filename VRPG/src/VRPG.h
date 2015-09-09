#ifndef VRPG_H_
#define VRPG_H_

#include "gameplay.h"
#include "world.h"

using namespace gameplay;

/**
 * Main game class.
 */
class VRPG: public Game
{
public:

    /**
     * Constructor.
     */
    VRPG();

    /**
     * @see Game::keyEvent
     */
	void keyEvent(Keyboard::KeyEvent evt, int key);
	
    /**
     * @see Game::touchEvent
     */
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

protected:

    /**
     * @see Game::initialize
     */
    void initialize();

    /**
     * @see Game::finalize
     */
    void finalize();

    /**
     * @see Game::update
     */
    void update(float elapsedTime);

    /**
     * @see Game::render
     */
    void render(float elapsedTime);

private:

    /**
     * Draws the scene each frame.
     */
    bool drawScene(Node* node);

	Node * createWorldNode(Mesh * mesh);

    Scene* _scene;
    Node * _group2;
	Light* _light;
	Node* _lightNode;
	Mesh * _cubeMesh;
	Node* _cameraNode;
	Mesh* _worldMesh;
	Node* _worldNode;
	Model* _worldModel;
	Material * _material;
	bool _wireframe;

	World * _world;

	void initWorld();
};

#endif
