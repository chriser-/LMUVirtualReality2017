//#define WIN32 1

#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <iostream>
#include <ios>

#include <OpenSG/OSGGLUT.h>
#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGGLUTWindow.h>
#include <OpenSG/OSGMultiDisplayWindow.h>

#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGAction.h>
#include <OpenSG/OSGIntersectAction.h>
#include <OpenSG/OSGTriangleIterator.h>
#include <OpenSG/OSGNameAttachment.h>

#include <OSGCSM/OSGCAVESceneManager.h>
#include <OSGCSM/OSGCAVEConfig.h>
#include <OSGCSM/appctrl.h>

#include <inVRs/SystemCore/UserDatabase/UserDatabase.h>
#include <inVRs/SystemCore/SystemCore.h>
#include <inVRs/SystemCore/Configuration.h>

#include <vrpn_Tracker.h>
#include <vrpn_Button.h>
#include <vrpn_Analog.h>
#include "Time.h"
#include "Game.h"

OSG_USING_NAMESPACE
OSG_USING_STD_NAMESPACE

OSGCSM::CAVEConfig cfg;
OSGCSM::CAVESceneManager *mgr = nullptr;
vrpn_Tracker_Remote* tracker =  nullptr;
vrpn_Button_Remote* button = nullptr;
vrpn_Analog_Remote* analog = nullptr;
Game* game = nullptr;

void cleanup()
{
	delete mgr;
	delete tracker;
	delete button;
	delete analog;
	delete game;
}

void print_tracker();

template<typename T>
T scale_tracker2cm(const T& value)
{
	static const float scale = OSGCSM::convert_length(cfg.getUnits(), 1.f, OSGCSM::CAVEConfig::CAVEUnitCentimeters);
	return value * scale;
}

Quaternion head_orientation = Quaternion(Vec3f(0.f, 1.f, 0.f), 3.141f);
Vec3f head_position = Vec3f(0.f, 170.f, 0);	// a 1.7m Person 2m in front of the scene

void VRPN_CALLBACK callback_head_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	head_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	head_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
}

Quaternion wand_orientation = Quaternion(Vec3f(0,0,0), 0);
Vec3f wand_position = Vec3f(0.f, 150.f, 0);
void moveObjectWithWand(NodeTransitPtr);
void VRPN_CALLBACK callback_wand_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	wand_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	wand_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
	if(game != nullptr)
	{
		moveObjectWithWand(NodeTransitPtr(game->m_debugWand.node()));
	}
}

auto analog_values = Vec3f();
void VRPN_CALLBACK callback_analog(void* userData, const vrpn_ANALOGCB analog)
{
	if (analog.num_channel >= 2)
		analog_values = Vec3f(analog.channel[0], 0, -analog.channel[1]);
}
void VRPN_CALLBACK callback_button(void* userData, const vrpn_BUTTONCB button)
{
	//if (button.button == 0 && button.state == 1)
	//	print_tracker();

	if (button.button == 0 && button.state == 1)
	{
		Line l;

		// point 1 is the position of the wand
		// point 2 is the position+direction of the wand
		Vec3f point2;
		wand_orientation.multVec(Vec3f(0, 0, -1), point2);
		point2 *= 5000; // length of 5000
		point2 += wand_position;
		l.setValue(wand_position, point2);

		std::cout << "From " << l.getPosition()
			<< ", dir " << l.getDirection() << std::endl;

		IntersectActionRefPtr act = IntersectAction::create();

		act->setLine(l);
		act->apply(game->GetRootNode().get());

		// did we hit something?
		if (act->didHit())
		{
			std::cout << "something was hit" << std::endl;
			std::string hitNodeName = "N/A";
			NodeRecPtr hitNode = act->getHitObject();
			while (hitNode != nullptr)
			{
				if(getName(hitNode))
				{
					hitNodeName = getName(hitNode);
				}
				GameObject* hitObject = game->GetBehavior(hitNode);
				std::cout << " object " << hitNode
					<< " type " << hitNode->getTypeName()
					<< " name " << hitNodeName
					<< " pos " << act->getHitPoint()
					<< std::endl;
				if (hitObject == nullptr)
				{
					hitNode = hitNode->getParent();
				}
				else
				{
					hitObject->OnHit();
					break;
				}
			}
		}
		else
		{
			std::cout << "nothing was hit" << std::endl;
		}

		// free the action
		act = nullptr;

		glutPostRedisplay();
	}
}


/*
* By Tobias Weiher
*/

Vec3f moveObjectWithWandTranslateOnly()
{
	Vec3f caveTrans = mgr->getTranslation();
	Real32 caveRot = mgr->getYRotate();

	Vec3f realWandTrans, virtualWandTrans;

	realWandTrans = wand_position;								// get vector of CAVEs Real Origin to Wand Real Position
	Quaternion rotWandRtoV = Quaternion(Vec3f(0, 1, 0), caveRot);	// rotate Wand around CAVE origin with amount of CAVEs orientation
	rotWandRtoV.multVec(realWandTrans, virtualWandTrans); 		// Real Wand now rotated into Virtual Space, offset to CAVE still required
	virtualWandTrans += caveTrans;								// added Offset, so that Wand is in CAVEs Virtual Space

	return virtualWandTrans;
}

Quaternion moveObjectWithWandRotateOnly()
{
	Quaternion realWandRot, virtualWandRot;
	Real32 caveRot = mgr->getYRotate();

	realWandRot = wand_orientation;								// get current Real Space Wand orientation
	virtualWandRot = Quaternion(Vec3f(0, 1, 0), caveRot);			// get the needed rotation of the CAVE in Virtual Space
	virtualWandRot.mult(realWandRot);							// rotate Wand finally into CAVEs Virtual Space Orientation

	return virtualWandRot;
}

void moveObjectWithWand(NodeTransitPtr componentTransformNode)
{
	if (!componentTransformNode->getCore()->getType().isDerivedFrom(ComponentTransform::getClassType()))
	{
		std::cout << "No ComponentTransformCore found!" << std::endl;
		return;
	}

	ComponentTransformRecPtr trans = dynamic_cast<ComponentTransform*>(componentTransformNode->getCore());

	Vec3f virtualWandTrans = moveObjectWithWandTranslateOnly();
	Quaternion virtualWandRot = moveObjectWithWandRotateOnly();

	trans->setTranslation(virtualWandTrans);
	trans->setRotation(virtualWandRot);
}

void InitTracker(OSGCSM::CAVEConfig &cfg)
{
	try
	{
		const char* const vrpn_name = "DTrack@localhost";
		tracker = new vrpn_Tracker_Remote(vrpn_name);
		tracker->shutup = true;
		tracker->register_change_handler(NULL, callback_head_tracker, cfg.getSensorIDHead());
		tracker->register_change_handler(NULL, callback_wand_tracker, cfg.getSensorIDController());
		button = new vrpn_Button_Remote(vrpn_name);
		button->shutup = true;
		button->register_change_handler(nullptr, callback_button);
		analog = new vrpn_Analog_Remote(vrpn_name);
		analog->shutup = true;
		analog->register_change_handler(NULL, callback_analog);
	}
	catch(const std::exception& e) 
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return;
	}
}

void check_tracker()
{
	tracker->mainloop();
	button->mainloop();
	analog->mainloop();
}

void print_tracker()
{
	Vec3f head_orientation_euler, wand_orientation_euler;
	head_orientation.getEulerAngleDeg(head_orientation_euler);
	wand_orientation.getEulerAngleDeg(wand_orientation_euler);
	std::cout << "Head position: " << head_position << " orientation: " << head_orientation_euler << '\n';
	std::cout << "Wand position: " << wand_position << " orientation: " << wand_orientation_euler << '\n';
	std::cout << "Analog: " << analog_values << '\n';
}

Vec2f mouse_last;
float speed = 1;
void mouseMove(int x, int y)
{
	Vec2f currentMouseVec = Vec2f(x, y);
	Vec2f mouseMove = mouse_last - currentMouseVec;
	std::cout << "Mouse Move: (" << mouseMove.x() << "," << mouseMove.y() << ")\n";
	NodeRefPtr root = mgr->getRoot();
	if(root != nullptr)
	{
		Vec3f* eulerAngles;
		head_orientation.getEulerAngleDeg(*eulerAngles);
		std::cout << "Euler Angles " << eulerAngles << std::endl;
		head_orientation.setValueAsAxisDeg(Vec3f(1,0,0) ,eulerAngles->x() + mouseMove.x() * speed);
		head_orientation.setValueAsAxisDeg(Vec3f(0,1,0) ,eulerAngles->y() + mouseMove.y() * speed);

		mouse_last = Vec2f(currentMouseVec);
		commitChanges();
		mgr->redraw();
	}
}

void mouse(int button, int state, int x, int y)
{
	if(button == 3)
	{
		game->Scroll(1);
	}
	else if(button == 4)
	{
		game->Scroll(-1);	
	}
	std::cout << "speed: " << speed << std::endl;
}

void keyboard(unsigned char k, int x, int y)
{
	Real32 ed;
	Vec3f wandRotation;
	switch(k)
	{
		case 'q':
		case 27: 
			cleanup();
			//exit(EXIT_SUCCESS);
			break;
		case 'w':
			wand_orientation.getEulerAngleDeg(wandRotation);
			wand_orientation.setValueAsAxisDeg(Vec3f(1, 0, 0), wandRotation.x() + 1);
			game->UpdateWand(wand_position, wand_orientation);
			print_tracker();
			break;
		case 's':
			wand_orientation.getEulerAngleDeg(wandRotation);
			wand_orientation.setValueAsAxisDeg(Vec3f(1, 0, 0), wandRotation.x() - 1);
			game->UpdateWand(wand_position, wand_orientation);
			print_tracker();
			break;
		case 'e':
			ed = mgr->getEyeSeparation() * .9f;
			std::cout << "Eye distance: " << ed << '\n';
			mgr->setEyeSeparation(ed);
			break;
		case 'E':
			ed = mgr->getEyeSeparation() * 1.1f;
			std::cout << "Eye distance: " << ed << '\n';
			mgr->setEyeSeparation(ed);
			break;
		case 'h':
			cfg.setFollowHead(!cfg.getFollowHead());
			std::cout << "following head: " << std::boolalpha << cfg.getFollowHead() << '\n';
			break;
		case 'i':
			print_tracker();
			break;
		case ' ':
			vrpn_BUTTONCB button;
			button.button = 0;
			button.state = 1;
			callback_button(nullptr, button);
		break;
		default:
			std::cout << "Key '" << k << "' ignored\n";
	}
}

void limitFPS(int) {
    glutPostRedisplay();
    glutTimerFunc(1000/60, limitFPS, 0);
}

void setupGLUT(int *argc, char *argv[])
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_RGB  |GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("OpenSG CSMDemo with VRPN API");
	glutDisplayFunc([]()
	{
                MyTime::UpdateDeltaTime();
		game->Update();
		// black navigation window
		glClear(GL_COLOR_BUFFER_BIT);
		glutSwapBuffers();
	});
	glutReshapeFunc([](int w, int h)
	{
		mgr->resize(w, h);
		glutPostRedisplay();
	});
	glutKeyboardFunc(keyboard);
	//glutPassiveMotionFunc(mouseMove);
	glutMouseFunc(mouse);
	glutIdleFunc([]()
	{
		check_tracker();
		commitChanges();
		mgr->setUserTransform(head_position, head_orientation);
		mgr->redraw();
		// the changelist should be cleared - else things could be copied multiple times
		OSG::Thread::getCurrentChangeList()->clear();
	});
}

int main(int argc, char **argv)
{

	OSG::preloadSharedObject("OSGFileIO");
	OSG::preloadSharedObject("OSGImageFileIO");

	try
	{
		bool cfgIsSet = false;
		NodeRefPtr scene = nullptr;

		// ChangeList needs to be set for OpenSG 1.4
		ChangeList::setReadWriteDefault();
		osgInit(argc,argv);

		// evaluate intial params
		for(int a=1 ; a<argc ; ++a)
		{
			if( argv[a][0] == '-' )
			{
				if ( strcmp(argv[a],"-f") == 0 ) 
				{
					char* cfgFile = argv[a][2] ? &argv[a][2] : &argv[++a][0];
					if (!cfg.loadFile(cfgFile)) 
					{
						std::cout << "ERROR: could not load config file '" << cfgFile << "'\n";
						return EXIT_FAILURE;
					}
					cfgIsSet = true;
				}
			} else {
				std::cout << "Loading scene file '" << argv[a] << "'\n";
				scene = SceneFileHandler::the()->read(argv[a], NULL);
			}
		}

		// load the CAVE setup config file if it was not loaded already:
		if (!cfgIsSet) 
		{
			const char* const default_config_filename = "config/mono.csm";
			if (!cfg.loadFile(default_config_filename)) 
			{
				std::cout << "ERROR: could not load default config file '" << default_config_filename << "'\n";
				return EXIT_FAILURE;
			}
		}

		cfg.printConfig();

		// start servers for video rendering
		if ( startServers(cfg) < 0 ) 
		{
			std::cout << "ERROR: Failed to start servers\n";
			return EXIT_FAILURE;
		}

		setupGLUT(&argc, argv);

		InitTracker(cfg);

		MultiDisplayWindowRefPtr mwin = createAppWindow(cfg, cfg.getBroadcastaddress());

		// very first step: load the configuration of the file structures, basically
		// paths are set. The Configuration always has to be loaded first since each
		// module uses the paths set in the configuration-file
		if (!Configuration::loadConfig("final/config/general.xml")) {
			printf("Error: could not load config-file!\n");
		}
		// register callbacks
		//InputInterface::registerModuleInitCallback(initInputInterface);
		//SystemCore::registerModuleInitCallback(initModules);
		//SystemCore::registerCoreComponentInitCallback(initCoreComponents);
		std::string systemCoreConfigFile = Configuration::getString(
			"SystemCore.systemCoreConfiguration");
		std::string outputInterfaceConfigFile = Configuration::getString(
			"Interfaces.outputInterfaceConfiguration");
		// in addition to the SystemCore config file, modules and interfaces config
		// files have to be loaded.
		std::string modulesConfigFile = Configuration::getString(
			"Modules.modulesConfiguration");
		std::string inputInterfaceConfigFile = Configuration::getString(
			"Interfaces.inputInterfaceConfiguration");

		if (SystemCore::configure(systemCoreConfigFile, outputInterfaceConfigFile, inputInterfaceConfigFile,
			modulesConfigFile)) {
			printf("Error: failed to setup SystemCore!\n");
			printf("Please check if the Plugins-path is correctly set to the inVRs-lib directory in the ");
			printf("'final/config/general.xml' config file, e.g.:\n");
			printf("<path name=\"Plugins\" path=\"/home/guest/inVRs/lib\"/>\n");
		}

		if (!game) {
			game = new Game();
		}
		commitChanges();

		mgr = new OSGCSM::CAVESceneManager(&cfg);
		mgr->setWindow(mwin);
		NodeRecPtr rootNode = game->GetRootNode();
		mgr->setRoot(rootNode);
		mgr->showAll();
		mgr->getWindow()->init();
		mgr->turnWandOff();
	}
	catch(const std::exception& e)
	{
		std::cout << "ERROR: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
	limitFPS(0);
	glutMainLoop();
}
