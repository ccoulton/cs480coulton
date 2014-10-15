#define GLM_FORCE_RADIANS
#include "modelClasses.h"
#include <iostream>
#include <GL/glew.h> 
#include <GL/glut.h>
#include <Magick++.h>
#include <string>

using namespace std;

Texture::Texture(GLenum TextureTarget, const std::string& FileName)
	{
	mtextureTarget = TextureTarget;
	mfileName = FileName;
	}

bool Texture::Load(){
	try{
		mimage.read(mfileName);
		mimage.write(&mblob, "RGBA");
		}
	catch (Magick::Error& Error){
		cout<<"Didn't load texture"<<mfileName<<Error.what()<<endl;
		return false;
		}
	glGenTextures(1, &mtextureObj);
	glBindTexture(mtextureTarget, mtextureObj);
	glTexImage2D(mtextureTarget, 0, GL_RGBA, mimage.columns(), mimage.rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mblob.data());
	glTexParameterf(mtextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(mtextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(mtextureTarget, 0);
	return true;
	}

void Texture::Bind(GLenum TextureUnit){
	glActiveTexture(TextureUnit);
	glBindTexture(mtextureTarget, mtextureObj);
	} 
	
<<<<<<< HEAD
Object::Object()
    {
    
    }
=======
>>>>>>> e61a61e35d7c282a10a022494c6d57f96f60cfab

bool Object::bind(int index)
    {
    glBindBuffer(GL_ARRAY_BUFFER, mesh[index].bufferName);
    Texs[index].bind(GL_TEXTURE0) 
     return true;
    }

void Object::render()
    {
     // something
    }

bool Object::load(std::string objName)
    {
	Assimp::Importer importer; //sets up assimp
	const aiScene *scene = importer.ReadFile(objName, aiProcess_Triangulate);  //reads from file
	mesh = new meshData[scene->mNumMeshes];  //make mesh array the size of meshes in file
	numMesh = scene->mNumMeshes;  //
	const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
	for(unsigned int meshindex =0; meshindex < numMesh; meshindex++){
	    //set up mesh array for number of meshes in scene since we trianglated vertiecs are already in face order
	    mesh[meshindex].NumVert = scene->mMeshes[meshindex]->mNumVertices;
		mesh[meshindex].Geo = new Vertex[mesh[meshindex.NumVert]];
	    
	    const aiMaterial* mat = scene->mMaterials[scene->mMeshes[meshindex]->mMaterialIndex];
	    
	    if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0){ //does the mesh have a material
	    	aiString Path;
	    	if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {  //get path of the texture
	    		std::string fullpath = Path.data;
	    		mesh[meshindex].Texs = new Texture(GL_TEXTURE_2D, fullpath.c_str());
	    		if (mesh[meshindex].Texs->Load())
	    			cout<<"Success"<<endl;
	    		else
	    		    return false;
	    		}
	    	}
	    for (unsigned int Index = 0; Index < mesh[meshindex].NumVert; Index++){
	    	const aiVector3D* pTexCoords = scene->mMeshes[meshindex]-> HasTextureCoords(0) ? &(scene->mMeshes[meshindex]->mTextureCoords[0][Index]): &Zero3D;  //puts in texture cords or empty if none exist from texture tut 16
		    mesh[meshindex].Geo[Index]
		    				  ={{scene->mMeshes[meshindex]->mVertices[Index].x,
		                     	 scene->mMeshes[meshindex]->mVertices[Index].y,
		                         scene->mMeshes[meshindex]->mVertices[Index].z}
		                       ,{pTexCoords->x, -(pTexCoords->y)}}; //add texture
		   	}
	glGenBuffers(1, &mesh[meshindex].bufferName);
    glBindBuffer(GL_ARRAY_BUFFER, mesh[meshindex].bufferName);
    glBufferData(GL_ARRAY_BUFFER, mesh[meshindex].NumVert*24,
                mesh[meshindex].Geo, GL_STATIC_DRAW);

	}
    return true;
    }

void Object::tick(float dt)
    {
     static float orbitAngle = 0;
     static float spinAngle = 0;

     orbitAngle += planetData.revolution * dt;
     spinAngle += planetData.selfSpin * dt;

     // add orbit tilt
     modelMatrix = glm::rotate(glm::mat4(1.0f), planetData.revolutionTilt, glm::vec3(0.0,0.0,1.0));

     // orbit position
     modelMatrix = glm::translate(modelMatrix, glm::vec3(planetData.revolutionRadius * sin(orbitAngle), 0.0, planetData.revolutionRadius * cos(orbitAngle)));

     // axis tilt
     modelMatrix = glm::rotate(modelMatrix, planetData.axisTilt, glm::vec3(1.0,0.0,0.0));

     // self rotation
     modelMatrix = glm::rotate(modelMatrix, spinAngle, glm::vec3(0.0,1.0,0.0));

     // scale planet size
     modelMatrix = glm::scale(modelMatrix, glm::vec3(planetData.radius));


    }
