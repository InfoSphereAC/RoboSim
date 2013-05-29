/*
 *  Texture.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdexcept>

#ifdef ANDROID_NDK
#include <asset_manager.h>
#endif

/*!
 * @abstract Loads and manages an OpenGL texture.
 * @discussion A very simple class to wrap an OpenGL texture. It supports only
 * what is needed here, which is GL_TEXTURE_2D in RGB or RGBA form.
 *
 * The textures are read in Targa (.tga) format, mainly because of all file
 * formats Photoshop exports, this is one of the most simple to read. It only
 * supports a subset of Targa files. In particular, they cannot use an image ID,
 * can only have BGR or BGRA data types and a pixel depth of 24 (BGR) or 32
 * (BGRA) bits per pixel. Most data of the file other than height, width, and
 * pixel data is completely ignored. The file gets closed when the texture is
 * loaded and can then be modified in any way without any effect on this object.
 *
 * This class has an implicit reference to an OpenGL context (there is no
 * portable way to make this explicit). All methods of the class, including
 * the destructor, have to be called while the same OpenGL context is current.
 * that was current during the execution of the constructor, or one that shares
 * its resources with that one.
 */
class Texture
{
	unsigned identifier;
public:
	/*!
	 * @abstract Creates a texture from a file.
	 * @param filename The path to the texture file.
	 * @throws std::runtime_error If the file cannot be opened or does not
	 * conform to the restrictions mentioned above.
	 */
#ifdef ANDROID_NDK
	Texture(AndroidAssetManager *manager, const char *filename) throw (std::runtime_error);
#else
	Texture(const char *filename) throw (std::runtime_error);
#endif
	
	/*!
	 * @abstract Destructor
	 * @discussion Deletes the OpenGL texture associated with this.
	 */
	~Texture() throw();
	
	/*!
	 * @abstract Makes this texture the current texture for the next draw calls.
	 */
	void set() const throw();
};
