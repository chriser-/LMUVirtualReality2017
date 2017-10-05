#include "Mesh.h"
#include <OSGSceneFileHandler.h>
#include <OSGGeometry.h>
#include <OSGSimpleTexturedMaterial.h>

Mesh::Mesh(OSG::NodeRecPtr parentNode, std::string name)
{
	std::string filename = "models/" + name + "/mesh.obj";
	std::cout << "Reading mesh file " << filename << std::endl;
	m_Node = OSG::SceneFileHandler::the()->read(filename.c_str());

	OSG::GeometryRecPtr meshGeo = dynamic_cast<OSG::Geometry*>(m_Node->getCore());
	if(meshGeo != nullptr)
	{
		OSG::SimpleTexturedMaterialRecPtr material = OSG::SimpleTexturedMaterial::create();
		OSG::ImageRecPtr texture = OSG::Image::create();
		texture->read(std::string("models/" + name + "/diffuse.tga").c_str());
		material->setImage(texture);
		meshGeo->setMaterial(material);
	}

	parentNode->addChild(m_Node);
}
