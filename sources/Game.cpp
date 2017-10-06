#include "Game.h"
#include "bird.h"
#include <OSGComponentTransform.h>
#include <OSGComponentTransformBase.h>
#include <OpenSG/OSGSceneGraphUtils.h>  // header for SceneGraphPrinter
#include <OSGSimpleGeometry.h>
#include <OSGNameAttachment.h>
#include <inVRs/SystemCore/UserDatabase/UserDatabase.h>
#include <mutex>
#include "Mesh.h"
#include <GL/glut.h>
#include <OSGDirectionalLight.h>


OSG_USING_STD_NAMESPACE
OSG_USING_NAMESPACE


void Game::AddBehavior(GameObject* behavior)
{
	std::cout << "Adding Behavior " << behavior->GetName() << " " << GameObject::hash_value(*behavior) << " (node: " << behavior->GetTransform().node() << ")" << std::endl;
	m_behaviors[behavior->GetTransform().node()] = behavior;
	// add behavior to root node
	if(behavior->GetName().compare("Bird") == 0)
	{
		m_root.node()->getChild(0)->addChild(behavior->GetTransform().node());
	}
	else
	{
		m_root.node()->addChild(behavior->GetTransform().node());
	}
}

void Game::RemoveBehavior(GameObject* behavior)
{
	std::cout << "Removing Behavior " << behavior->GetName() << " (" << GameObject::hash_value(*behavior) << ")" << std::endl;
	const NodeRecPtr nodeToDelete = behavior->GetTransform().node();
	if (behavior->GetName() == "Bird")
	{
		m_birdsAlive--;
		m_root.node()->getChild(0)->subChild(nodeToDelete);
	}
	else
	{
		m_root.node()->subChild(nodeToDelete);
	}
	m_behaviorsToDelete.insert(behavior->GetTransform().node());
}

Gun* Game::GetGun()
{
	return m_gun;
}

GameObject* Game::GetBehavior(NodeRecPtr fromNode)
{
	const auto iter = m_behaviors.find(fromNode);
	if(iter != m_behaviors.end())
	{
		return iter->second;
	}
	return nullptr;
}

SpriteAtlas* Game::GetSpriteAtlas(std::string spriteAtlasName)
{
	const auto iter = m_SpriteAtlasses.find(spriteAtlasName);
	if(iter != m_SpriteAtlasses.end())
	{
		return iter->second;
	}
	SpriteAtlas* atlas = new SpriteAtlas(spriteAtlasName);
	m_SpriteAtlasses[spriteAtlasName] = atlas;
	return atlas;
}

GameObject* makeEnv(Vec3f position, Vec3f rotation, Vec3f scale, int sortIndex, std::string name)
{
	GameObject* obj = new GameObject(name);
	obj->GetTransform()->setTranslation(position);
	Quaternion rot;
	rot.setValue(osgDegree2Rad(rotation.x()), osgDegree2Rad(rotation.y()), osgDegree2Rad(rotation.z()));
	obj->GetTransform()->setRotation(rot);
	obj->GetTransform()->setScale(scale);
	Sprite* objSprite = new Sprite(obj->GetTransform().node(), name, "objects", sortIndex);
	obj->AddComponent(objSprite);
	Vec2f dimensions = objSprite->GetDimensions();
	obj->Translate(Vec3f(0, scale.y() * (dimensions.y() / 2), 0));
	return obj;
}

GameObject* makeTree(Vec3f position, Vec3f rotation, int sortIndex)
{
	return makeEnv(position, rotation, Vec3f(0.5, 0.5, 0.5), sortIndex, "Tree");
}

GameObject* makeGrass(Vec3f position, Vec3f rotation, int sortIndex)
{
	return makeEnv(position, rotation, Vec3f(0.25, 0.25, 0.25), sortIndex, "Grass");
}

GameObject* makeBird(std::string name, int speed)
{
	Vec3f pos = Vec3f((rand() % 400) - 200, 20, -100);
	Bird* bird = new Bird("Bird", name);
	bird->SetSpeed(speed);
	bird->Translate(pos);
	return bird;
}

Game* Game::m_gameInstance;
GameObject* m_goToModify = nullptr;
Game::Game()
{
	m_gameInstance = this;
	m_root = ComponentTransformNodeRefPtr::create();
	m_root->setScale(Vec3f(7,7,7));
	m_root->setTranslation(Vec3f(0, 0, -1000));
	m_root.node()->addChild(GroupNodeRefPtr::create()); // bird root (hack for collision)
	setName(m_root.node(), "Root");

	// floor
	GameObject* floor = new GameObject("Floor");
	GeometryRecPtr floorGeo = makePlaneGeo(50000, 50000, 1000, 1000);
	TextureObjChunkRecPtr tex = TextureObjChunk::create();
	tex->setImage(GetSpriteAtlas("objects")->GetImage("ground").get());
	tex->setWrapS(GL_REPEAT);
	tex->setWrapT(GL_REPEAT);
	TextureEnvChunkRefPtr texEnv = TextureEnvChunk::create();
	texEnv->setEnvMode(GL_MODULATE);
	ChunkMaterialRecPtr mat = ChunkMaterial::create();
	mat->addChunk(tex);
	mat->addChunk(texEnv);

	//SimpleTexturedMaterialRecPtr floorMat = SimpleTexturedMaterial::create();
	//floorMat->setImage(GetSpriteAtlas("objects")->GetImage("ground").get());
	floorGeo->setMaterial(mat);
	NodeRecPtr floorNode = Node::create();
	floorNode->setCore(floorGeo);
	floor->GetTransform().node()->addChild(floorNode);
	floor->GetTransform()->setRotation(Quaternion(Vec3f(1, 0, 0), osgDegree2Rad(90)));

	// skybox
	User* localUser = UserDatabase::getLocalUser();
	CameraTransformation* camera = localUser->getCamera();

	std::string skyPath = "skybox/";
	m_skybox.init(20, 20, 20, 10000, (skyPath + "lostatseaday/lostatseaday_dn.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_up.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_ft.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_bk.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_rt.jpg").c_str(),
		(skyPath + "lostatseaday/lostatseaday_lf.jpg").c_str());
	m_root.node()->addChild(m_skybox.getNodePtr());
	m_skybox.setupRender(camera->getPosition());

	// trees
	// front row
	makeTree(Vec3f(-40, 0, -100), Vec3f(0, 0, 0), 1);
	makeTree(Vec3f(200, 0, -56), Vec3f(0, -30, 0), 1);
	
	// back row
	makeTree(Vec3f(-258, 0, -184), Vec3f(0, 30, 0), -2);
	makeTree(Vec3f(164, 0, -242), Vec3f(0, -30, 0), -2);

	// grass
	// front row
	makeGrass(Vec3f(-210, 0, -44), Vec3f(0, 30, 0), 2);
	makeGrass(Vec3f(0, 0, -100), Vec3f(0, 0, 0), 2);
	makeGrass(Vec3f(210, 0, -44), Vec3f(0, -30, 0), 2);

	// back row
	makeGrass(Vec3f(-210, 0, -214), Vec3f(0, 30, 0), -1);
	makeGrass(Vec3f(0, 0, -270), Vec3f(0, 0, 0), -1);
	makeGrass(Vec3f(210, 0, -214), Vec3f(0, -30, 0), -1);

	// gun
	m_gun = new Gun();
	m_goToModify = m_gun;

	// light
	DirectionalLightNodeRefPtr light = DirectionalLightNodeRefPtr::create();
	light->setOn(true);
	light->setDirection(0, -1, 0);
	m_root.node()->addChild(light);

	// open scene TODO
	m_birdsAlive = 0;
	m_currentWave = 0;
	m_gameReady = true;
}

void Game::Update()
{
	// main loop
	if(m_gameReady)
	{
		if(m_birdsAlive <= 0)
		{
			m_currentWave++;
			m_birdsAlive = 1 + m_currentWave * 2;
			m_gun->SetRemainingShots(m_birdsAlive + 3); // 3 more shots than birds alive
			for (int i = 0; i < m_birdsAlive; ++i)
			{
				std::string duckName = "DuckBlue";
				int random = rand() % 100;
				if(random < 33)
				{
					duckName = "DuckRed";
				}
				else if(random < 66)
				{
					duckName = "DuckBlack";
				}
				makeBird(duckName, 20 + (10 * m_currentWave));
			}
		}
	}

	// Update all behaviors
	for (auto& behavior : m_behaviors)
	{
		//std::cout << "Updating first " << behavior.first << " second " << behavior.second << std::endl;
		behavior.second->Update();
	}
	for(auto& behaviorToDelete : m_behaviorsToDelete)
	{
		m_behaviors.erase(behaviorToDelete);
	}
	m_behaviorsToDelete.clear();
}

void Game::UpdateWand(Vec3f position, Quaternion orientation) const
{
	m_gun->GetTransform()->setTranslation(Vec3f(position.x(), position.y()-142.f, position.z()+122.f));
	m_gun->GetTransform()->setRotation(orientation);
}

Game::~Game()
{
	// Delete all behaviors
	for (auto& behavior : m_behaviors)
	{
		RemoveBehavior(behavior.second);
	}
	for (auto& behaviorToDelete : m_behaviorsToDelete)
	{
		m_behaviors.erase(behaviorToDelete);
	}
	m_behaviorsToDelete.clear();
}

void Game::Scroll(int direction) const
{
	if (m_goToModify != nullptr) 
	{

		//if (glutGetModifiers() & GLUT_ACTIVE_SHIFT)
		//{
		//	m_goToModify->Translate(Vec3f(0, 0, direction));
		//}
		//else if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
		//{
		//	m_goToModify->Translate(Vec3f(direction, 0, 0));
		//}
		//else
		//{
			m_goToModify->Translate(Vec3f(0, direction, 0));
		//}
		std::cout << m_goToModify->GetTransform()->getTranslation() << std::endl;
	}
}

NodeTransitPtr Game::GetRootNode() const
{
	return NodeTransitPtr(m_root.node());
}

Game* Game::Instance()
{
	return m_gameInstance;
}
