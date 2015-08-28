#ifndef VRPG_H_
#define VRPG_H_

#include "gameplay.h"

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

	Node * createCube(int x, int y, int z);
	Node * createWorldNode(Mesh * mesh);

    Scene* _scene;
	Node * _group1;
	Node * _cubeNode;
	Node * _cubeNode2;
	Light* _light;
	Node* _lightNode;
	Mesh * _cubeMesh;
	Node* _cameraNode;
    bool _wireframe;
};

#endif
