#include "Mesh.h"

void Mesh::InitMesh()
{
	//Vertices
	glGenBuffers(1, (GLuint*)&(verticesId));
	glBindBuffer(GL_ARRAY_BUFFER, verticesId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * verticesCount * 3, vertices, GL_STATIC_DRAW);

	//Indices
	glGenBuffers(1, (GLuint*)&(indicesId));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indicesCount, indices, GL_STATIC_DRAW);

	//Normals
	glGenBuffers(1, (GLuint*)&(normalsId));
	glBindBuffer(GL_ARRAY_BUFFER, normalsId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * normalsCount * 3, &normals[0], GL_STATIC_DRAW);

	//Texture Coords
	glGenBuffers(1, (GLuint*)&(texCoordsId));
	glBindBuffer(GL_ARRAY_BUFFER, texCoordsId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * texCoordsCount * 2, &texCoords[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::DrawMesh(GLuint textureID, bool drawTextures, bool wireframe, bool shadedWireframe)
{
	if (!shadedWireframe)
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glColor3f(diffuseColor.x, diffuseColor.y, diffuseColor.z);

	if (drawTextures && !wireframe && textureID != 0)
	{
		glBindTexture(GL_TEXTURE_2D, textureID);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordsId);
		glTexCoordPointer(2, GL_FLOAT, 0, NULL);
	}
	else
	{
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, verticesId);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	if (normalsCount > 0)
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, normalsId);
		glNormalPointer(GL_FLOAT, 0, NULL);
	}
	else
	{
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesId);
	glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	if (shadedWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glColor3f(0.0f, 1.0f, 0.0f);

		glEnableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, verticesId);
		glVertexPointer(3, GL_FLOAT, 0, NULL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesId);
		glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, NULL);

		glPopAttrib();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Mesh::DrawNormals(bool vertexNormals, bool faceNormals, float vertexNormalLength, float faceNormalLength, glm::vec3 vertexNormalColor, glm::vec3 faceNormalColor)
{
	if (vertexNormals && verticesCount > 0 && normalsCount > 0)
	{
		glColor3f(vertexNormalColor.x, vertexNormalColor.y, vertexNormalColor.z);
		glBegin(GL_LINES);
		for (size_t i = 0; i < verticesCount; i++)
		{
			glVertex3f(vertices[i * 3], vertices[i * 3 + 1], vertices[i * 3 + 2]);
			glVertex3f(vertices[i * 3] + normals[i * 3] * vertexNormalLength,
				vertices[i * 3 + 1] + normals[i * 3 + 1] * vertexNormalLength,
				vertices[i * 3 + 2] + normals[i * 3 + 2] * vertexNormalLength
			);
		}
		glEnd();
	}

	if (faceNormals && indicesCount > 0)
	{
		glColor3f(faceNormalColor.x, faceNormalColor.y, faceNormalColor.z);

		glBegin(GL_LINES);
		for (size_t i = 0; i < indicesCount; i += 3)
		{
			uint index0 = indices[i];
			uint index1 = indices[i + 1];
			uint index2 = indices[i + 2];

			glm::vec3 v0(vertices[index0 * 3], vertices[index0 * 3 + 1], vertices[index0 * 3 + 2]);
			glm::vec3 v1(vertices[index1 * 3], vertices[index1 * 3 + 1], vertices[index1 * 3 + 2]);
			glm::vec3 v2(vertices[index2 * 3], vertices[index2 * 3 + 1], vertices[index2 * 3 + 2]);

			glm::vec3 edge1 = v1 - v0;
			glm::vec3 edge2 = v2 - v0;
			glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

			glm::vec3 faceCenter = (v0 + v1 + v2) / 3.0f;

			glVertex3f(faceCenter.x, faceCenter.y, faceCenter.z);
			glVertex3f(faceCenter.x + normal.x * faceNormalLength,
				faceCenter.y + normal.y * faceNormalLength,
				faceCenter.z + normal.z * faceNormalLength
			);
		}
		glEnd();
	}

	glColor3f(1.0f, 1.0f, 1.0f);
}

void Mesh::CleanUpMesh()
{
	glDeleteBuffers(1, &verticesId);
	glDeleteBuffers(1, &indicesId);
	glDeleteBuffers(1, &normalsId);
	glDeleteBuffers(1, &texCoordsId);

	delete[] vertices;
	delete[] indices;
	delete[] normals;
	delete[] texCoords;

	vertices = nullptr;
	indices = nullptr;
	normals = nullptr;
	texCoords = nullptr;
}