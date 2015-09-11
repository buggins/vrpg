#include "VRPG.h"
#include "world.h"
#include "blocks.h"
#include "logger.h"

#define USE_SPOT_LIGHT 0

// Declare our game instance
VRPG game;

VRPG::VRPG()
    : _scene(NULL), _wireframe(false)
{
	runWorldUnitTests();
}

Material * createMaterialBlocks();

#define VERTEX_COMPONENTS 11

static float face_vertices_north[VERTEX_COMPONENTS * 4] =
{
	-0.5, 0.5, -0.5,	0.0, 0.0, -1.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, 0.5, -0.5,		0.0, 0.0, -1.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, -0.5, -0.5,	0.0, 0.0, -1.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, -0.5, -0.5,	0.0, 0.0, -1.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static float face_vertices_south[VERTEX_COMPONENTS * 4] =
{
	-0.5, -0.5, 0.5,	0.0, 0.0, 1.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, -0.5, 0.5,		0.0, 0.0, 1.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, 0.5, 0.5,		0.0, 0.0, 1.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, 0.5, 0.5,		0.0, 0.0, 1.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static float face_vertices_west[VERTEX_COMPONENTS * 4] =
{
	-0.5, -0.5, -0.5,	-1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	-0.5, -0.5, 0.5,	-1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, 0.5, -0.5,	-1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	-0.5, 0.5, 0.5,		-1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		1.0, 1.0
};

static float face_vertices_east[VERTEX_COMPONENTS * 4] =
{
	0.5, -0.5, 0.5,		1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, -0.5, -0.5,	1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	0.5, 0.5, 0.5,		1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, 0.5, -0.5,		1.0, 0.0, 0.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static float face_vertices_up[VERTEX_COMPONENTS * 4] =
{
	-0.5, 0.5, 0.5,		0.0, 1.0, 0.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, 0.5, 0.5,		0.0, 1.0, 0.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, 0.5, -0.5,	0.0, 1.0, 0.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, 0.5, -0.5,		0.0, 1.0, 0.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static float face_vertices_down[VERTEX_COMPONENTS * 4] =
{
	-0.5, -0.5, -0.5,	0.0, -1.0, 0.0,		1.0, 1.0, 1.0,		0.0, 0.0,
	0.5, -0.5, -0.5,	0.0, -1.0, 0.0,		1.0, 1.0, 1.0,		1.0, 0.0,
	-0.5, -0.5, 0.5,	0.0, -1.0, 0.0,		1.0, 1.0, 1.0,		0.0, 1.0,
	0.5, -0.5, 0.5,		0.0, -1.0, 0.0,		1.0, 1.0, 1.0,		1.0, 1.0,
};

static int face_indexes[6] =
{
    0, 1, 2, 2, 1, 3
};

static int face_indexes_back[6] =
{
    0, 2, 1, 2, 3, 1
};

static void fillFaceMesh(float * data, float * src, float x0, float y0, float z0, int tileX, int tileY) {
	for (int i = 0; i < 4; i++) {
		float * srcvertex = src + i * VERTEX_COMPONENTS;
		float * dstvertex = data + i * VERTEX_COMPONENTS;
		for (int j = 0; j < 11; j++) {
			float v = srcvertex[j];
			switch (j) {
			case 0: // x
				v += x0;
				break;
			case 1: // y
				v += y0;
				break;
			case 2: // z
				v += z0;
				break;
			case 9: // tx.u
				v = ((tileX + v * BLOCK_SPRITE_SIZE)) / (float)BLOCK_TEXTURE_DX;
				break;
			case 10: // tx.v
				v = (BLOCK_TEXTURE_DY - (tileY + v * BLOCK_SPRITE_SIZE)) / (float)BLOCK_TEXTURE_DY;
				break;
			}
			dstvertex[j] = v;
		}
	}
}

static void createFaceMesh(float * data, Dir face, float x0, float y0, float z0, int tileIndex) {

	int tileX = (tileIndex % BLOCK_TEXTURE_SPRITES_PER_LINE) * BLOCK_SPRITE_STEP + BLOCK_SPRITE_OFFSET;
	int tileY = (tileIndex / BLOCK_TEXTURE_SPRITES_PER_LINE) * BLOCK_SPRITE_STEP + BLOCK_SPRITE_OFFSET;
	// data is 11 comp * 4 vert floats
	switch (face) {
    default:
	case NORTH:
		fillFaceMesh(data, face_vertices_north, x0, y0, z0, tileX, tileY);
		break;
	case SOUTH:
		fillFaceMesh(data, face_vertices_south, x0, y0, z0, tileX, tileY);
		break;
    case WEST:
		fillFaceMesh(data, face_vertices_west, x0, y0, z0, tileX, tileY);
		break;
    case EAST:
		fillFaceMesh(data, face_vertices_east, x0, y0, z0, tileX, tileY);
		break;
	case UP:
		fillFaceMesh(data, face_vertices_up, x0, y0, z0, tileX, tileY);
		break;
	case DOWN:
		fillFaceMesh(data, face_vertices_down, x0, y0, z0, tileX, tileY);
		break;
	}
}

class MeshVisitor : public CellVisitor {
	FloatArray vertices;
	IntArray indexes;
	int faceCount;
	FILE * log;
	lUInt64 startTime;
public:
	MeshVisitor() : faceCount(0), log(NULL) {
		log = fopen("faces.log", "wt");
		startTime = GetCurrentTimeMillis();
	}
	~MeshVisitor() {
		if (log) {
			fprintf(log, "Time elapsed: %lld millis\n", GetCurrentTimeMillis() - startTime);
			fclose(log);
		}
	}
	virtual void newDirection(Position & camPosition) {
		//fprintf(log, "Cam position : %d,%d,%d \t dir=%d\n", camPosition.pos.x, camPosition.pos.y, camPosition.pos.z, camPosition.direction.dir);
	}
	virtual void visitFace(World * world, Position & camPosition, Vector3d pos, cell_t cell, Dir face) {
		//fprintf(log, "face %d: %d,%d,%d \t %d \t %d\n", faceCount, pos.x, pos.y, pos.z, cell, face);
		int v0 = faceCount * 4;
		faceCount++;
		float * vptr = vertices.append(0.0f, 11 * 4);
		int * iptr = indexes.append(0, 6);
		BlockDef * def = BLOCK_DEFS + cell;
		int texIndex = def->txIndex;
		createFaceMesh(vptr, face, pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f, texIndex);
        for (int i = 0; i < 6; i++)
            iptr[i] = v0 + face_indexes[i];
//        float vbuf[8*4];
//        int ibuf[6];
//        createFaceMesh(vbuf, face, pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f);
//        for (int i = 0; i < 6; i++)
//            ibuf[i] = v0 + face_indexes[i];
	}
	virtual void visit(World * world, Position & camPosition, Vector3d pos, cell_t cell, int visibleFaces) {
		//visibleFaces = 63;
		for (int i = 0; i < 6; i++)
			if (visibleFaces & (1<<i))
				visitFace(world, camPosition, pos, cell, (Dir)i);
	}

	Mesh* createMesh() {
		unsigned int vertexCount = faceCount * 4;
		unsigned int indexCount = faceCount * 6;
		VertexFormat::Element elements[] =
		{
			VertexFormat::Element(VertexFormat::POSITION, 3),
			VertexFormat::Element(VertexFormat::NORMAL, 3),
			VertexFormat::Element(VertexFormat::COLOR, 3),
			VertexFormat::Element(VertexFormat::TEXCOORD0, 2)
		};
		Mesh* mesh = Mesh::createMesh(VertexFormat(elements, 4), vertexCount, false);
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


Material * createMaterialBlocks() {
#if USE_SPOT_LIGHT_LIGHT==1
	Material* material = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag", "SPOT_LIGHT_COUNT 1");
#else
	//SPECULAR;
	Material* material = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag", "VERTEX_COLOR;POINT_LIGHT_COUNT 1");
#endif
	if (material == NULL)
	{
		GP_ERROR("Failed to create material for model.");
		return NULL;
	}
	// These parameters are normally set in a .material file but this example sets them programmatically.
	// Bind the uniform "u_worldViewProjectionMatrix" to use the WORLD_VIEW_PROJECTION_MATRIX from the scene's active camera and the node that the model belongs to.
	material->setParameterAutoBinding("u_worldViewProjectionMatrix", "WORLD_VIEW_PROJECTION_MATRIX");
	material->setParameterAutoBinding("u_inverseTransposeWorldViewMatrix", "INVERSE_TRANSPOSE_WORLD_VIEW_MATRIX");
	material->setParameterAutoBinding("u_cameraPosition", "CAMERA_VIEW_POSITION");
	material->setParameterAutoBinding("u_worldViewMatrix", "WORLD_VIEW_MATRIX");
	//u_specularExponent = 50
	material->getParameter("u_specularExponent")->setValue(50.0f);
	// Set the ambient color of the material.
	material->getParameter("u_ambientColor")->setValue(Vector3(0.2f, 0.2f, 0.2f));
	//u_ambientColor = SCENE_AMBIENT_COLOR

	// Bind the light's color and direction to the material.

	// Load the texture from file.
	Texture::Sampler* sampler = material->getParameter("u_diffuseTexture")->setValue(BLOCK_TEXTURE_FILENAME, true);
	sampler->setFilterMode(Texture::NEAREST_MIPMAP_LINEAR, Texture::NEAREST);
	material->getStateBlock()->setCullFace(true);//true
	material->getStateBlock()->setDepthTest(true);
	material->getStateBlock()->setDepthWrite(true);
	//material->getStateBlock()->set
	return material;
}

static int cubeIndex = 1;

Node * VRPG::createWorldNode(Mesh * mesh) {


	Model* cubeModel = Model::create(mesh);
	Node * cubeNode = Node::create("world");
	cubeModel->setMaterial(_material);
	cubeNode->setDrawable(cubeModel);
	//material->release();
	//SAFE_RELEASE(material);
	//cubeModel->release();
	SAFE_RELEASE(cubeModel);
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

void VRPG::initWorld() {
	World * world = new World();

	world->getCamPosition().pos = Vector3d(0, 3, 0);
	world->getCamPosition().direction.set(EAST);

	world->setCell(-5, 3, 5, 1);
	world->setCell(-2, 1, -7, 8);
	world->setCell(-2, 2, -7, 8);
	world->setCell(-2, 3, -7, 8);
	world->setCell(-20, 7, 4, 8);
	world->setCell(20, 6, 9, 8);
	world->setCell(5, 7, -15, 8);
	world->setCell(5, 7, 15, 8);
#if 0
	world->setCell(0, 7, 0, 1);
	world->setCell(5, 7, 0, 1);
	world->setCell(5, 7, 5, 1);
	world->setCell(-5, 7, -5, 1);
	world->setCell(-5, 7, 5, 1);
#else
	for (int x = -100; x <= 100; x++) {
		for (int z = -100; z <= 100; z++) {
			world->setCell(x, 0, z, 3);
		}
	}
	for (int x = -10; x <= 10; x++) {
		for (int z = -10; z <= 10; z++) {
			if (z < -2 || z > 2 || x < -2 || x > 2) {
				world->setCell(x, 8, z, 1);
			}
		}
	}
	for (int x = -10; x <= 10; x++) {
		world->setCell(x, 1, -10, 2);
		world->setCell(x, 2, -10, 2);
		world->setCell(x, 3, -10, 2);
		world->setCell(x, 1, 10, 2);
		world->setCell(x, 2, 10, 2);
		world->setCell(x, 3, 10, 2);
		world->setCell(11, 1, x, 2);
		world->setCell(11, 2, x, 2);
		world->setCell(11, 3, x, 2);
		world->setCell(11, 4, x, 2);
		world->setCell(-11, 1, x, 2);
		world->setCell(-11, 2, x, 2);
		world->setCell(-11, 3, x, 2);
		world->setCell(-11, 4, x, 2);
	}
	for (int i = 0; i < 10; i++) {
		world->setCell(4, 1 + i, 5 + i, 3);
		world->setCell(5, 1 + i, 5 + i, 3);
		world->setCell(6, 1 + i, 5 + i, 3);
		world->setCell(4, 1 + i, 6 + i, 3);
		world->setCell(5, 1 + i, 6 + i, 3);
		world->setCell(6, 1 + i, 6 + i, 3);
	}
	for (int i = 0; i < 5; i++) {
		world->setCell(-6, 1, -6 + i * 2, 7);
		world->setCell(-6, 7, -6 + i * 2, 7);
		world->setCell(-6 + i * 2, 1, -6, 7);
		world->setCell(-6 + i * 2, 7, -6, 7);
	}
#endif
	world->setCell(3, 0, -6, 0); // hole
	world->setCell(3, 0, -7, 0); // hole
	world->setCell(4, 0, -6, 0); // hole
	world->setCell(4, 0, -7, 0); // hole

	world->setCell(2, 2, 0, 8);

	_world = world;
}

void VRPG::initialize()
{
	CRLog::setFileLogger("vrpg.log", true);
	CRLog::setLogLevel(CRLog::LL_TRACE);
	CRLog::info("VRPG::initialize()");

	_material = createMaterialBlocks();

	CRLog::trace("initWorld()");
	initWorld();

	// Create a new empty scene.
	_scene = Scene::create();

	// Create the camera.
	Camera* camera = Camera::createPerspective(45.0f, getAspectRatio(), 0.2f, MAX_VIEW_DISTANCE + 1);
	Node* cameraNode = _scene->addNode("camera");
	_cameraNode = cameraNode;

	// Attach the camera to a node. This determines the position of the camera.
	cameraNode->setCamera(camera);

	// Make this the active camera of the scene.
	_scene->setActiveCamera(camera);
	SAFE_RELEASE(camera);

	// Move the camera to look at the origin.
	//cameraNode->translate(0, 5, 0);
	//cameraNode->rotateY(MATH_DEG_TO_RAD(45.25f));

	// Create a white light.
	//Light* light = Light::createDirectional(0.75f, 0.75f, 0.75f);

#if	USE_SPOT_LIGHT==1
	Light* light = Light::createSpot(1.5f, 0.75f, 0.75f, 5.0f, MATH_DEG_TO_RAD(60.0f), MATH_DEG_TO_RAD(90.0f));
#else
	Light* light = Light::createPoint(0.8f, 0.8f, 0.8f, 12.0f);
#endif
	Node* lightNode = _scene->addNode("light");
	lightNode->setLight(light);
	//lightNode->translate(0, 5, 0);
	//lightNode->rotateX(MATH_DEG_TO_RAD(-5.25f));
	//lightNode->rotateY(MATH_DEG_TO_RAD(45.25f));
	_light = light;
	_lightNode = lightNode;
	// Release the light because the node now holds a reference to it.
	SAFE_RELEASE(light);
	//lightNode->rotateX(MATH_DEG_TO_RAD(-25.0f));


#if	USE_SPOT_LIGHT==1
	material->getParameter("u_spotLightColor[0]")->setValue(_lightNode->getLight()->getColor());
	material->getParameter("u_spotLightInnerAngleCos[0]")->setValue(_lightNode->getLight()->getInnerAngleCos());
	material->getParameter("u_spotLightOuterAngleCos[0]")->setValue(_lightNode->getLight()->getOuterAngleCos());
	material->getParameter("u_spotLightRangeInverse[0]")->setValue(_lightNode->getLight()->getRangeInverse());
	material->getParameter("u_spotLightDirection[0]")->bindValue(_lightNode, &Node::getForwardVectorView);
	material->getParameter("u_spotLightPosition[0]")->bindValue(_lightNode, &Node::getTranslationView);
#else
	_material->getParameter("u_pointLightColor[0]")->setValue(_lightNode->getLight()->getColor());
	_material->getParameter("u_pointLightPosition[0]")->bindValue(_lightNode, &Node::getForwardVectorWorld);
	_material->getParameter("u_pointLightRangeInverse[0]")->bindValue(_lightNode->getLight(), &Light::getRangeInverse);
#endif


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



	//TestVisitor * visitor = new TestVisitor();
	////world->visitVisibleCells(world->getCamPosition(), visitor);
	//world->visitVisibleCellsAllDirections(world->getCamPosition(), visitor);
	//
	//delete visitor;

	MeshVisitor * meshVisitor = new MeshVisitor();
	//world->visitVisibleCells(world->getCamPosition(), meshVisitor);
	_world->visitVisibleCellsAllDirectionsFast(_world->getCamPosition(), meshVisitor);
	Mesh * worldMesh = meshVisitor->createMesh();
	_worldMesh = worldMesh;
	_worldNode = createWorldNode(worldMesh);
	SAFE_RELEASE(worldMesh);
	delete meshVisitor;

	_group2->addChild(_worldNode);

}

void VRPG::finalize()
{
    SAFE_RELEASE(_scene);
	SAFE_RELEASE(_material);
	delete _world;
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
	if (animation) {
		//_cameraNode->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 15000.0f * 180.0f));
		//_lightNode->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 22356.0f * 180.0f));
		//_lightNode->rotateX(MATH_DEG_TO_RAD((float)elapsedTime / 62356.0f * 180.0f));
		//_group2->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 5000.0f * 180.0f));
	}
}

void VRPG::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

	_cameraNode->setRotation(Vector3(1, 0, 0), 0);
	_lightNode->setRotation(Vector3(1, 0, 0), 0);

	switch (_world->getCamPosition().direction.dir) {
	case WEST:
		_cameraNode->rotateY(MATH_DEG_TO_RAD(90));
		_lightNode->rotateY(MATH_DEG_TO_RAD(90));
		break;
	case EAST:
		_cameraNode->rotateY(MATH_DEG_TO_RAD(-90));
		_lightNode->rotateY(MATH_DEG_TO_RAD(-90));
		break;
	case NORTH:
		break;
	case SOUTH:
		_cameraNode->rotateY(MATH_DEG_TO_RAD(180));
		_lightNode->rotateY(MATH_DEG_TO_RAD(180));
		break;
    default:
        break;
	}

	Position p = _world->getCamPosition();
	//p.pos -= p.direction.forward;
	_cameraNode->setTranslation(p.pos.x, p.pos.y, p.pos.z);
	_lightNode->setTranslation(p.pos.x, p.pos.y, p.pos.z);
	_cameraNode->translate(0.5, 0.5, 0.5);
	//_cameraNode->translate(-0.5, -0.5, -0.5);
	//_lightNode->translate(-0.5, -0.5, -0.5);

	//_cameraNode->rotateX(MATH_DEG_TO_RAD(-10));

#define REVISIT_EACH_RENDER 1

	MeshVisitor * meshVisitor = new MeshVisitor();
	_world->visitVisibleCellsAllDirectionsFast(_world->getCamPosition(), meshVisitor);
#if REVISIT_EACH_RENDER==1
	Mesh * oldMesh = _worldMesh;
	Mesh * worldMesh = meshVisitor->createMesh();
	_worldMesh = worldMesh;
	//delete oldMesh;
#endif

	Node * worldNode = createWorldNode(_worldMesh);

#if REVISIT_EACH_RENDER==1
	SAFE_RELEASE(worldMesh);
#endif
	delete meshVisitor;

	_group2->removeAllChildren();

	_group2->addChild(worldNode);

	SAFE_RELEASE(worldNode);

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

static const char * dir_names[] = {
	"NORTH",
	"SOUTH",
	"WEST",
	"EAST",
	"UP",
	"DOWN",
};

void VRPG::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (evt == Keyboard::KEY_PRESS)
    {
		Position * pos = &_world->getCamPosition();
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
		case Keyboard::KEY_W:
			pos->pos += pos->direction.forward;
			break;
		case Keyboard::KEY_S:
			pos->pos -= pos->direction.forward;
			break;
		case Keyboard::KEY_A:
			pos->pos += pos->direction.left;
			break;
		case Keyboard::KEY_D:
			pos->pos += pos->direction.right;
			break;
		case Keyboard::KEY_Q:
			pos->direction.turnLeft();
			break;
		case Keyboard::KEY_E:
			pos->direction.turnRight();
			break;
		case Keyboard::KEY_Z:
			pos->pos += pos->direction.down;
			break;
		case Keyboard::KEY_C:
			pos->pos += pos->direction.up;
			break;
		}
		CRLog::trace("Position: %d,%d,%d direction: %s", pos->pos.x, pos->pos.y, pos->pos.z, dir_names[pos->direction.dir]);
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
