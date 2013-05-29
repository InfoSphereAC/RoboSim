/*
 *  Model.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <istream>
#include <map>
#include <vector>
#include <stdexcept>
#include <string>

#ifdef ANDROID_NDK
#include <asset_manager.h>
#endif

class Drawer;
class Texture;

/*!
 * @abstract A 3D Model for drawing
 * @discussion This contains all the data for drawing a 3D model, which is
 * here always read simply from an Wavefront (aka .obj) file. It does not
 * support sub-models, animation or anything else along these lines, it is just
 * a bunch of vertex data, material information and a texture.
 * Implicitly, the Model is associated with an OpenGL context (there is no
 * portable way to make this association explicit, sadly). All of the models
 * methods (including constructor and destructor) have to be called while the
 * same OpenGL context is active. Otherwise, the results are undefined but very
 * likely to be crashing.
 * Only a subset of the full .obj-Format is supported. In particular, the file
 * may only contain triangles and cannot have more than one material, but has to
 * have vertex, normal and texture coordinate data for all entries. Also,
 * negative numbers for references are not supported. These limitations are all
 * based on just being able to read the files that the simulator uses, although
 * the class ought to be useful enough for other uses as well.
 */
class Model
{
	unsigned vertexBuffer;
	unsigned indexBuffer;
	unsigned numElements;
	
	float ambient[4];
	float diffuse[4];
	float emissive[4];
	float specular[4];
	float shininess;
	
	Texture *texture;
	
	void parseFloatVector(const char *, std::vector<float> &values, unsigned number) throw();
	void parseFace(std::istream &stream, const std::vector<float> &vertices, const std::vector<float> &normals, const std::vector<float> &texCoords, std::map<std::string, unsigned short> &unifiedVertexIndices, std::vector<float> &vertexData, std::vector<unsigned short> &indexData) throw(std::range_error);
	void parseMaterialLibrary(std::istream &stream, Drawer *drawer) throw(std::runtime_error);
	void loadTexture(std::istream &stream, Drawer *drawer) throw(std::runtime_error);
	
public:
	/*!
	 * @abstract Creates a new Model
	 * @discussion The model is read from an .obj file and its associated .mtl
	 * file. For simplicity, the path to the .mtl file is resolved relative to
	 * the current working directory, NOT relative to the name passed in here.
	 * As long as all files are in the working directory (which is the case in
	 * this simulator), that will not be a problem.
	 * The Drawer is just used to get the texture, so that various models do not
	 * cause the same image file to be loaded multiple times.
	 * @param objFilename The path to the .obj file.
	 * @param 
	 * @throws std::runtime_error If the file specified cannot be opened or
	 * the material library it references cannot be opened or the texture
	 * cannot be opened, or contains something invalid.
	 */
	Model(const char *objFilename, Drawer *drawer) throw(std::runtime_error);
	
	/*! Destructor */
	~Model() throw();
	
	/*!
	 * @abstract Sets up the material data, vertex buffers and vertex pointers.
	 * @discussion This sets up the necessary state for drawing the model. It
	 * does not draw it yet. The idea is that if you want to draw one model
	 * several times directly after each other, you call setup(), and then draw()
	 * as often as you want. If you are drawing it only once, setupAndDraw() is
	 * probably the better choice.
	 */
	void setup() throw();
	/*!
	 * @abstract Draws the model, assuming that the state is already set.
	 * @discussion This just issues the plain OpenGL draw call. Any other state,
	 * such as vertex pointers, VBOs, texture and material have to be set up in
	 * advance by setup() for this model and the current OpenGL Context. If not,
	 * the program will likely crash.
	 */
	void draw() throw();
	 
	/*!
	 * @abstract Draws the entire model.
	 * @discussion This sets up the graphics state and then draws the model. It
	 * is equivalent to calling setup(); draw(); and does not depend on anything
	 * else being done (except for the conditions of the entire class).
	 */
	void setupAndDraw() throw();
};
