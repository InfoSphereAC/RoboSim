/*
 *  Model.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Model.h"

#include <map>
#include <fstream>
#include <sstream>

#include "Drawer.h"
#include "OpenGL.h"
#include "Texture.h"

#ifdef ANDROID_NDK
#include "android_ifstream.h"
#endif

/*
 * Some explanation regarding this code:
 *
 * An OBJ file contains two key pieces of information: First Vertex data, which
 * specifies where what corner is as well the texture coordinate and surface
 * normal at that position. Second, there is face data, which specifies what
 * vertices form triangles. A single vertex can have multiple different normals
 * and texture coordinates, depending on what face it is a part of (for example
 * at a corner where multiple textured regions come together).
 *
 * The OBJ format allows for that, by giving each corner of a face as a triple
 * like 1/2/1, being the index of the vertex, of the texture coordinate set and
 * of the surface normal at that place. OpenGL, however, does not allow that:
 * Here, an corner is always a single number referring to all three. So if both
 * 1/2/1 and 1/3/2 are used in the file, they have to be two separate indices
 * here, which only works by copying the position of the first vertex. That and
 * finding what triple has been mapped to what number are the most important
 * things this file does.
 */

// ----------------
// Helper functions
// ----------------

void Model::parseFloatVector(const char *line, std::vector<float> &values, unsigned number) throw()
{
	if (number == 2)
	{
		float vals[2];
		sscanf(line, "%*s %f %f", &vals[0], &vals[1]);
		values.push_back(vals[0]);
		values.push_back(vals[1]);
	}
	else if (number == 3)
	{
		float vals[3];
		sscanf(line, "%*s %f %f %f", &vals[0], &vals[1], &vals[2]);
		values.push_back(vals[0]);
		values.push_back(vals[1]);
		values.push_back(vals[2]);
	}
}

// ---------
// Main work
// ---------

void Model::parseFace(std::istream &stream, const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, std::map<std::string, unsigned short> &unifiedVertexIndices, std::vector<float> &vertexData, std::vector<unsigned short> &indexData) throw(std::range_error)
{
	std::string indices;
	for (unsigned i = 0; i < 3; i++) // We only deal with triangels
	{
		stream >> indices;
		std::map<std::string, unsigned short>::iterator index = unifiedVertexIndices.find(indices);
		if (index == unifiedVertexIndices.end())
		{
			unsigned vertex;
			unsigned textureCoord;
			unsigned normal;
			sscanf(indices.c_str(), "%u/%u/%u", &vertex, &textureCoord, &normal);
			
			// OBJ-file indexing starts at 1, not 0
			vertex -= 1;
			textureCoord -= 1;
			normal -= 1;
			
			if (vertex >= vertices.size()) throw std::range_error("Vertex index out of range.");
			if (normal >= normals.size()) throw std::range_error("Surface normal index out of range.");
			if (textureCoord >= texCoords.size()) throw std::range_error("Texture coordinate index out of range.");
			
			indexData.push_back(vertexData.size() / 8);
			
			vertexData.push_back(vertices[vertex*3+0]);
			vertexData.push_back(vertices[vertex*3+1]);
			vertexData.push_back(vertices[vertex*3+2]);
			
			vertexData.push_back(normals[normal*3+0]);
			vertexData.push_back(normals[normal*3+1]);
			vertexData.push_back(normals[normal*3+2]);
			
			vertexData.push_back(texCoords[textureCoord*2+0]);
			vertexData.push_back(texCoords[textureCoord*2+1]);
		}
		else indexData.push_back(index->second);
	}
}

// -------------
// Material data
// -------------

void Model::parseMaterialLibrary(std::istream &linestream, Drawer *drawer) throw(std::runtime_error)
{
	std::string filename;
	linestream >> filename;
	
#ifdef ANDROID_NDK
	android_ifstream mtlFile(drawer->getAssetManager(), filename.c_str());
#else
	std::ifstream mtlFile(filename.c_str());
#endif
	if (!mtlFile) throw std::runtime_error("Cannot open material library file.");
	
	diffuse[3] = 1.0f;
	ambient[3] = 1.0f;
	specular[3] = 1.0f;
	emissive[3] = 1.0f;
	
	while (mtlFile.good())
	{
		std::string line;
		std::getline(mtlFile, line);
		std::istringstream linestream(line);
		std::string token;
		linestream >> token;
		
		if (token == "Kd")		sscanf(line.c_str(), "Kd %f %f %f", &diffuse[0], &diffuse[1], &diffuse[2]);
		else if (token == "Ka")	sscanf(line.c_str(), "Ka %f %f %f", &ambient[0], &ambient[1], &ambient[2]);
		else if (token == "Ks")	sscanf(line.c_str(), "Ks %f %f %f", &specular[0], &specular[1], &specular[2]);
		else if (token == "Ns") sscanf(line.c_str(), "Ns %f", &shininess);
		else if (token == "map_Kd")
			loadTexture(linestream, drawer);
	}
}

inline void Model::loadTexture(std::istream &stream, Drawer *drawer) throw(std::runtime_error)
{
	std::string filename;
	stream >> filename;

	texture = drawer->textureWithFilename(filename);
}

// -------------------------------
// Initializiation and destruction
// -------------------------------

Model::Model(const char *objFilename, Drawer *drawer) throw(std::runtime_error) : texture(0)
{
#ifdef ANDROID_NDK
	android_ifstream objFile(drawer->getAssetManager(), objFilename);
#else
	std::ifstream objFile(objFilename);
#endif
	if (!objFile) throw std::runtime_error("Cannot open model file.");
	
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texCoords;
	
	std::map<std::string, unsigned short> unifiedVertexIndices;
	
	std::vector<float> vertexData;
	std::vector<unsigned short> indexData;
	
	while (objFile.good())
	{
		std::string line;
		std::getline(objFile, line);
		std::istringstream linestream(line);
		std::string token;
		linestream >> token;
		
		if (token == "v")
			parseFloatVector(line.c_str(), vertices, 3);
		else if (token == "vn")
			parseFloatVector(line.c_str(), normals, 3);
		else if (token == "vt")
			parseFloatVector(line.c_str(), texCoords, 2);
		else if (token == "f")
			parseFace(linestream, vertices, normals, texCoords, unifiedVertexIndices, vertexData, indexData);
		else if (token == "mtllib")
			parseMaterialLibrary(linestream, drawer);
	}
	
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), &(vertexData[0]), GL_STATIC_DRAW);
	
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(unsigned short), &(indexData[0]), GL_STATIC_DRAW);
	
	numElements = indexData.size();
}

Model::~Model() throw()
{
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);
	
	delete texture;
}

// -----------------
// Setup and drawing
// -----------------

void Model::setup() throw()
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexPointer(3, GL_FLOAT, sizeof(float [8]), reinterpret_cast<GLvoid *> (0));
	glNormalPointer(GL_FLOAT, sizeof(float [8]), reinterpret_cast<GLvoid *> (sizeof(float [3])));
	glTexCoordPointer(2, GL_FLOAT, sizeof(float [8]), reinterpret_cast<GLvoid *> (sizeof(float [6])));
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	
	texture->set();
}

void Model::draw() throw()
{
	glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0);
}

void Model::setupAndDraw() throw()
{
	setup();
	draw();
}
