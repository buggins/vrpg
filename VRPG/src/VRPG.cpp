#include "VRPG.h"
#include "world.h"

// Declare our game instance
VRPG game;

VRPG::VRPG()
    : _scene(NULL), _wireframe(false)
{
	runWorldUnitTests();
}

Material * createMaterial();

static float face_vertices_south[] =
{
	-0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 0.0,
	0.5, -0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 0.0,
	-0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 0.0, 1.0,
	0.5, 0.5, 0.5, 0.0, 0.0, 1.0, 1.0, 1.0,
};

static float face_vertices_up[] =
{
	-0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 0.0, 0.0,
	0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 1.0, 0.0,
	-0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 1.0,
	0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 1.0,
};

static float face_vertices_north[] =
{
	-0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 0.0,
	0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 1.0, 0.0,
	-0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 0.0, 1.0,
	0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 1.0, 1.0,
};

static float face_vertices_down[] =
{
	-0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 0.0, 0.0,
	0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 1.0, 0.0,
	-0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 0.0, 1.0,
	0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 1.0, 1.0,
};

static float face_vertices_east[] =
{
	0.5, -0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0,
	0.5, -0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.5, 1.0, 0.0, 0.0, 0.0, 1.0,
	0.5, 0.5, -0.5, 1.0, 0.0, 0.0, 1.0, 1.0,
};

static float face_vertices_west[] =
{
	-0.5, -0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 0.0,
	-0.5, -0.5, 0.5, -1.0, 0.0, 0.0, 1.0, 0.0,
	-0.5, 0.5, -0.5, -1.0, 0.0, 0.0, 0.0, 1.0,
	-0.5, 0.5, 0.5, -1.0, 0.0, 0.0, 1.0, 1.0
};

static int face_indexes[] =
{
    0, 1, 2, 2, 1, 3
};

static int face_indexes_back[] =
{
    0, 2, 1, 2, 3, 1
};

static void fillFaceMesh(float * data, float * src, float x0, float y0, float z0) {
	for (int i = 0; i < 4; i++) {
		float * srcvertex = src + i * 8;
		float * dstvertex = data + i * 8;
		for (int j = 0; j < 8; j++) {
			float v = srcvertex[j];
			if (j == 0)
				v += x0;
			else if (j == 1)
				v += y0;
			else if (j == 2)
				v += z0;
			dstvertex[j] = v;
		}
	}
}

static void createFaceMesh(float * data, Dir face, float x0, float y0, float z0) {
	// data is 8 comp * 4 vert floats
	switch (face) {
    default:
    case SOUTH:
	//case NORTH:
		fillFaceMesh(data, face_vertices_north, x0, y0, z0);
		break;
    case NORTH:
	//case SOUTH:
		fillFaceMesh(data, face_vertices_south, x0, y0, z0);
		break;
	//case EAST:
    case WEST:
		fillFaceMesh(data, face_vertices_west, x0, y0, z0);
		break;
	//case WEST:
    case EAST:
		fillFaceMesh(data, face_vertices_east, x0, y0, z0);
		break;
	case UP:
		fillFaceMesh(data, face_vertices_up, x0, y0, z0);
		break;
	case DOWN:
		fillFaceMesh(data, face_vertices_down, x0, y0, z0);
		break;
	}
}

class MeshVisitor : public CellVisitor {
	FloatArray vertices;
	IntArray indexes;
	int faceCount;
public:
	MeshVisitor() : faceCount(0) {
	}
	~MeshVisitor() {
	}
	virtual void visitFace(World * world, Position & camPosition, Vector3d pos, cell_t cell, Dir face) {
		int v0 = faceCount * 4;
		faceCount++;
		float * vptr = vertices.append(0.0f, 8 * 4);
		int * iptr = indexes.append(0, 6);
		createFaceMesh(vptr, face, pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f);
        for (int i = 0; i < 6; i++)
            iptr[i] = v0 + face_indexes[i];
//        float vbuf[8*4];
//        int ibuf[6];
//        createFaceMesh(vbuf, face, pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f);
//        for (int i = 0; i < 6; i++)
//            ibuf[i] = v0 + face_indexes[i];
	}
	Mesh* createMesh() {
		unsigned int vertexCount = faceCount * 4;
		unsigned int indexCount = faceCount * 6;
		VertexFormat::Element elements[] =
		{
			VertexFormat::Element(VertexFormat::POSITION, 3),
			VertexFormat::Element(VertexFormat::NORMAL, 3),
			VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
		};
		Mesh* mesh = Mesh::createMesh(VertexFormat(elements, 3), vertexCount, false);
		if (mesh == NULL)
		{
			GP_ERROR("Failed to create mesh.");
			return NULL;
		}
		mesh->setVertexData(vertices.ptr(), 0, vertexCount);
		MeshPart* meshPart = mesh->addPart(Mesh::TRIANGLES, Mesh::INDEX32, indexCount, false);
		meshPart->setIndexData(indexes.ptr(), 0, indexCount);
		return mesh;
	}
};


static Mesh* createCubeMesh(float size = 1.0f)
{
	float a = size * 0.5f;
	float vertices[] =
	{
		-a, -a, a, 0.0, 0.0, 1.0, 0.0, 0.0,
		a, -a, a, 0.0, 0.0, 1.0, 1.0, 0.0,
		-a, a, a, 0.0, 0.0, 1.0, 0.0, 1.0,
		a, a, a, 0.0, 0.0, 1.0, 1.0, 1.0,
		-a, a, a, 0.0, 1.0, 0.0, 0.0, 0.0,
		a, a, a, 0.0, 1.0, 0.0, 1.0, 0.0,
		-a, a, -a, 0.0, 1.0, 0.0, 0.0, 1.0,
		a, a, -a, 0.0, 1.0, 0.0, 1.0, 1.0,
		-a, a, -a, 0.0, 0.0, -1.0, 0.0, 0.0,
		a, a, -a, 0.0, 0.0, -1.0, 1.0, 0.0,
		-a, -a, -a, 0.0, 0.0, -1.0, 0.0, 1.0,
		a, -a, -a, 0.0, 0.0, -1.0, 1.0, 1.0,
		-a, -a, -a, 0.0, -1.0, 0.0, 0.0, 0.0,
		a, -a, -a, 0.0, -1.0, 0.0, 1.0, 0.0,
		-a, -a, a, 0.0, -1.0, 0.0, 0.0, 1.0,
		a, -a, a, 0.0, -1.0, 0.0, 1.0, 1.0,
		a, -a, a, 1.0, 0.0, 0.0, 0.0, 0.0,
		a, -a, -a, 1.0, 0.0, 0.0, 1.0, 0.0,
		a, a, a, 1.0, 0.0, 0.0, 0.0, 1.0,
		a, a, -a, 1.0, 0.0, 0.0, 1.0, 1.0,
		-a, -a, -a, -1.0, 0.0, 0.0, 0.0, 0.0,
		-a, -a, a, -1.0, 0.0, 0.0, 1.0, 0.0,
		-a, a, -a, -1.0, 0.0, 0.0, 0.0, 1.0,
		-a, a, a, -1.0, 0.0, 0.0, 1.0, 1.0
	};
	short indices[] =
	{
		0, 1, 2, 2, 1, 3,
        4, 5, 6, 6, 5, 7,
        8, 9, 10, 10, 9, 11,
        12, 13, 14, 14, 13, 15,
        16, 17, 18, 18, 17, 19,
        20, 21, 22, 22, 21, 23
	};
	unsigned int vertexCount = 24;
	unsigned int indexCount = 36;
	VertexFormat::Element elements[] =
	{
		VertexFormat::Element(VertexFormat::POSITION, 3),
		VertexFormat::Element(VertexFormat::NORMAL, 3),
		VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
	};
	Mesh* mesh = Mesh::createMesh(VertexFormat(elements, 3), vertexCount, false);
	if (mesh == NULL)
	{
		GP_ERROR("Failed to create mesh.");
		return NULL;
	}
	mesh->setVertexData(vertices, 0, vertexCount);
	MeshPart* meshPart = mesh->addPart(Mesh::TRIANGLES, Mesh::INDEX16, indexCount, false);
	meshPart->setIndexData(indices, 0, indexCount);
	return mesh;
}

Material * createMaterial() {
	Material* material = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag", "DIRECTIONAL_LIGHT_COUNT 1");
	if (material == NULL)
	{
		GP_ERROR("Failed to create material for model.");
		return NULL;
	}
	// These parameters are normally set in a .material file but this example sets them programmatically.
	// Bind the uniform "u_worldViewProjectionMatrix" to use the WORLD_VIEW_PROJECTION_MATRIX from the scene's active camera and the node that the model belongs to.
	material->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	material->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	// Set the ambient color of the material.
	material->getParameter("u_ambientColor")->setValue(Vector3(0.2f, 0.2f, 0.2f));

	// Bind the light's color and direction to the material.

	// Load the texture from file.
	Texture::Sampler* sampler = material->getParameter("u_diffuseTexture")->setValue("res/png/crate.png", true);
	sampler->setFilterMode(Texture::LINEAR_MIPMAP_LINEAR, Texture::LINEAR);
	material->getStateBlock()->setCullFace(true);
	material->getStateBlock()->setDepthTest(true);
	material->getStateBlock()->setDepthWrite(true);
	return material;
}

static int cubeIndex = 1;

Node * VRPG::createWorldNode(Mesh * mesh) {
	Material * material = createMaterial();
	material->getParameter("u_directionalLightColor[0]")->setValue(_lightNode->getLight()->getColor());
	material->getParameter("u_directionalLightDirection[0]")->bindValue(_lightNode, &Node::getForwardVectorWorld);

	Model* cubeModel = Model::create(mesh);
	Node * cubeNode = Node::create("world");
	cubeModel->setMaterial(material);
	cubeNode->setDrawable(cubeModel);
	return cubeNode;
}

Node * VRPG::createCube(int x, int y, int z) {
	Material * material = createMaterial();
	material->getParameter("u_directionalLightColor[0]")->setValue(_lightNode->getLight()->getColor());
	material->getParameter("u_directionalLightDirection[0]")->bindValue(_lightNode, &Node::getForwardVectorWorld);

	char nodeName[64];
	sprintf(nodeName, "cube%d", cubeIndex++);

	Model* cubeModel = Model::create(_cubeMesh);
	Node * cubeNode = Node::create(nodeName);
	cubeModel->setMaterial(material);
	cubeNode->setDrawable(cubeModel);
	cubeNode->setTranslationX(x);
	cubeNode->setTranslationY(y);
	cubeNode->setTranslationZ(z);
	return cubeNode;
}

class TestVisitor : public CellVisitor {
	FILE * f;
public:
	TestVisitor() {
		f = fopen("facelog.log", "wt");
	}
	~TestVisitor() {
		fclose(f);
	}
	virtual void visitFace(World * world, Position & camPosition, Vector3d pos, cell_t cell, Dir face) {
		fprintf(f, "pos %d,%d,%d  \tcell=%d  \tface=%d\n", pos.x, pos.y, pos.z, cell, face);
	}
};

void VRPG::initialize()
{
	// Create a new empty scene.
	_scene = Scene::create();

	// Create the camera.
	Camera* camera = Camera::createPerspective(45.0f, getAspectRatio(), 0.5f, 50.0f);
	Node* cameraNode = _scene->addNode("camera");
	_cameraNode = cameraNode;

	// Attach the camera to a node. This determines the position of the camera.
	cameraNode->setCamera(camera);

	// Make this the active camera of the scene.
	_scene->setActiveCamera(camera);
	SAFE_RELEASE(camera);

	// Move the camera to look at the origin.
	cameraNode->translate(0, 3, 0);
	cameraNode->rotateY(MATH_DEG_TO_RAD(45.25f));

	// Create a white light.
	Light* light = Light::createDirectional(0.75f, 0.75f, 0.75f);
	Node* lightNode = _scene->addNode("light");
	lightNode->setLight(light);
	_light = light;
	_lightNode = lightNode;
	// Release the light because the node now holds a reference to it.
	SAFE_RELEASE(light);
	lightNode->rotateX(MATH_DEG_TO_RAD(-45.0f));

	// Create the material for the cube model and assign it to the first mesh part.
	Material * material = createMaterial();
	material->getParameter("u_directionalLightColor[0]")->setValue(lightNode->getLight()->getColor());
	material->getParameter("u_directionalLightDirection[0]")->bindValue(lightNode, &Node::getForwardVectorWorld);

	// Create the cube mesh and model.
	Mesh* cubeMesh = createCubeMesh();
	_cubeMesh = cubeMesh;
	Model* cubeModel = Model::create(cubeMesh);
	// Release the mesh because the model now holds a reference to it.

	_group1 = _scene->addNode("group1");
	_cubeNode = createCube(0, 0, 0);
	_cubeNode->rotateY(MATH_PIOVER4);
	_group1->addChild(_cubeNode);
	_cubeNode2 = createCube(0, 2, 0);
	_cubeNode2->rotateY(MATH_PIOVER4 / 4);
	_cubeNode2->scaleX(0.5f);
	_cubeNode2->translateZ(0.5f);
	_group1->addChild(_cubeNode2);
	SAFE_RELEASE(cubeMesh);

	_group2 = _scene->addNode("group2");
#if 0
	int sz = 50;
	for (int x = -sz; x <= sz; x++) {
		for (int y = -sz; y <= sz; y++) {
			_group2->addChild(createCube(x, -2, y));
		}
		_group2->addChild(createCube(x, -1, -sz));
		_group2->addChild(createCube(x, -1, sz));
		_group2->addChild(createCube(x, 0, -sz));
		_group2->addChild(createCube(x, 0, sz));
		_group2->addChild(createCube(x, 1, -sz));
		_group2->addChild(createCube(x, 1, sz));
	}
#endif

	World * world = new World();

	world->getCamPosition().pos = Vector3d(0, 3, 3);
    world->getCamPosition().direction.set(NORTH);

    world->setCell(-5, 3, 5, 1);
	world->setCell(-2, 1, -7, 8);
	world->setCell(-2, 2, -7, 8);
	world->setCell(-2, 3, -7, 8);
#if 0
    world->setCell(0, 7, 0, 1);
    world->setCell(5, 7, 0, 1);
    world->setCell(5, 7, 5, 1);
    world->setCell(-5, 7, -5, 1);
    world->setCell(-5, 7, 5, 1);
#else
	for (int x = -10; x <= 10; x++)
		for (int z = -10; z <= 10; z++) {
			world->setCell(x, 0, z, 1);
			world->setCell(x, 7, z, 1);
		}
	for (int x = -10; x <= 10; x++) {
		world->setCell(x, 1, -10, 2);
		world->setCell(x, 1, 10, 2);
		world->setCell(-10, 1, x, 3);
		world->setCell(10, 1, x, 3);
	}
#endif
	world->setCell(3, 0, -6, 0); // hole
	world->setCell(3, 0, -7, 0); // hole
	world->setCell(4, 0, -6, 0); // hole
	world->setCell(4, 0, -7, 0); // hole
	TestVisitor * visitor = new TestVisitor();
	world->visitVisibleCells(world->getCamPosition(), visitor);
	delete visitor;

	MeshVisitor * meshVisitor = new MeshVisitor();
	world->visitVisibleCells(world->getCamPosition(), meshVisitor);
	Mesh * worldMesh = meshVisitor->createMesh();
	Node * worldNode = createWorldNode(worldMesh);
	SAFE_RELEASE(worldMesh);
	delete meshVisitor;

	_group2->addChild(worldNode);



	delete world;

}

void VRPG::finalize()
{
    SAFE_RELEASE(_scene);
}

static bool animation = false;

void VRPG::update(float elapsedTime)
{
    // Rotate model
    //_scene->findNode("box")
	//_group1->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
	//_cubeNode->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
	//_cameraNode->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 15000.0f * 180.0f));
	//_cubeNode2->rotateX(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
	if (animation)
		_group2->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 5000.0f * 180.0f));
}

void VRPG::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

    // Visit all the nodes in the scene for drawing
    _scene->visit(this, &VRPG::drawScene);
}

bool VRPG::drawScene(Node* node)
{
    // If the node visited contains a drawable object, draw it
    Drawable* drawable = node->getDrawable(); 
    if (drawable)
        drawable->draw(_wireframe);

    return true;
}

void VRPG::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (evt == Keyboard::KEY_PRESS)
    {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
        }
    }
}

void VRPG::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        _wireframe = !_wireframe;
		if (!_wireframe)
			animation = !animation;
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}
