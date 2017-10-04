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
Vec3f head_position = Vec3f(0.f, 170.f, 200.f);	// a 1.7m Person 2m in front of the scene

void VRPN_CALLBACK callback_head_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	head_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	head_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
}

Quaternion wand_orientation = Quaternion(Vec3f(0,0,0), 0);
Vec3f wand_position = Vec3f(0.f, 150.f, 200.f); // wand at 1.5m, 2m in fron of the scene
void VRPN_CALLBACK callback_wand_tracker(void* userData, const vrpn_TRACKERCB tracker)
{
	wand_orientation = Quaternion(tracker.quat[0], tracker.quat[1], tracker.quat[2], tracker.quat[3]);
	wand_position = Vec3f(scale_tracker2cm(Vec3d(tracker.pos)));
}

auto analog_values = Vec3f();
void VRPN_CALLBACK callback_analog(void* userData, const vrpn_ANALOGCB analog)
{
	if (analog.num_channel >= 2)
		analog_values = Vec3f(analog.channel[0], 0, -analog.channel[1]);
}

void VRPN_CALLBACK callback_button(void* userData, const vrpn_BUTTONCB button)
{
	if (button.button == 0 && button.state == 1)
		print_tracker();
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

GeoPnt3fPropertyRefPtr isectPoints;

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
			wand_orientation.setValueAsAxisDeg(Vec3f(0, 1, 0), wandRotation.y() + 1);
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
		case ' ':   // send a ray through the clicked pixel
					/*
					Intersection testing for rays is done using an
					IntersectAction. The ray itself is calculated by the
					SimpleSceneManager, given the clicked pixel.

					It needs to be set up with the line that is to be
					intersected. A line is a semi-infinite ray which has a
					starting point and a direction, and extends in the
					direction to infinity.

					To do the actual test the Action's apply() method is used.

					The results can be received from the Action. The main
					difference is if something was hit or not, which is
					returned in didHit().

					If an intersection did occur, the other data elements are
					valid, otherwise they are undefined.

					The information that is stored in the action is the object
					which was hit, the triangle of the object that was hit (in
					the form of its index) and the actual hit position.
					*/
		{
			Line l;

			// TODO: set line which matches wand orientation
			// point 1 is the position of the wand
			// point 2 is the position+direction of the wand
			Vec3f point2;
			wand_orientation.multVec(Vec3f(0, 0, 1), point2);
			point2 += wand_position;
			l.setValue(wand_position, point2);

			std::cout << "From " << l.getPosition()
				<< ", dir " << l.getDirection() << std::endl;

			IntersectActionRefPtr act = IntersectAction::create();

			act->setLine(l);
			act->apply(game->GetRootNode().get());

			isectPoints->setValue(l.getPosition(), 0);
			isectPoints->setValue(l.getPosition() + l.getDirection(), 1);


			// did we hit something?
			if (act->didHit())
			{
				std::cout << "something was hit" << std::endl;
				// yes!! print and highlight it
				std::cout << " object " << act->getHitObject()
					<< " tri " << act->getHitTriangle()
					<< " at " << act->getHitPoint();

				act->getHitObject();

				// stop the ray on the hit surface
				Pnt3f is = l.getPosition() +
					l.getDirection() * act->getHitT();

				isectPoints->setValue(is, 1);

				// find the triangle that was hit
				TriangleIterator it(act->getHitObject());
				it.seek(act->getHitTriangle());

				// Draw its normal at the intersection point
				isectPoints->setValue(is, 2);
				isectPoints->setValue(is + act->getHitNormal() * 5, 3);


				// calculate its vertex positions in world space
				Matrix m;
				act->getHitObject()->getToWorld(m);

				// and turn them into a triangle
				Pnt3f p = it.getPosition(0);
				m.mult(p, p);
				isectPoints->setValue(p, 4);
				p = it.getPosition(1);
				m.mult(p, p);
				isectPoints->setValue(p, 5);
				p = it.getPosition(2);
				m.mult(p, p);
				isectPoints->setValue(p, 6);
			}
			else
			{
				std::cout << "nothing was hit" << std::endl;
				// no, get rid of the triangle and highlight.
				isectPoints->setValue(Pnt3f(0, 0, 0), 2);
				isectPoints->setValue(Pnt3f(0, 0, 0), 3);
				isectPoints->setValue(Pnt3f(0, 0, 0), 4);

			}

			// free the action
			act = nullptr;

			std::cout << std::endl;

			glutPostRedisplay();
		}
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
		const auto speed = 1.f;
		mgr->setUserTransform(head_position, head_orientation);
		mgr->setTranslation(mgr->getTranslation() + speed * analog_values);
		commitChanges();
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

		isectPoints = OSG::GeoPnt3fProperty::create();
		isectPoints->addValue(OSG::Pnt3f(0, 0, 0));
		isectPoints->addValue(OSG::Pnt3f(0, 0, 0));
		isectPoints->addValue(OSG::Pnt3f(0, 0, 0));
		isectPoints->addValue(OSG::Pnt3f(0, 0, 0));
		isectPoints->addValue(OSG::Pnt3f(0, 0, 0));

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
