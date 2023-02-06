#include "vec.h"
#include <math.h>

struct Sphere {
    float radius;
    int sectors;
    int stacks;
    int interleavedStride; // Don't change this
    float* vertices;
    float* interleavedVertices;
    float* normals; // only used for internal stuff, we won't use textures
    float* texCoords; // same thing ^
    unsigned int* indices;
    unsigned int* lineIndices;
};

// should probably move these definitions to Sphere.h
struct Sphere* createSphere(float radius, int sectors, int stacks);
void printSelf(struct Sphere *mySphere);
unsigned int getVertexCount(struct Sphere *mySphere);
unsigned int getNormalCount(struct Sphere *mySphere);
unsigned int getTexCoordCount(struct Sphere *mySphere);
unsigned int getLineIndexCount(struct Sphere *mySphere);
unsigned int getTriangleCount(struct Sphere *mySphere);
unsigned int getVertexSize(struct Sphere *mySphere);
unsigned int getNormalSize(struct Sphere *mySphere);
unsigned int getTexCoordSize(struct Sphere *mySphere);
unsigned int getIndexSize(struct Sphere *mySphere);
unsigned int getLineIndexSize(struct Sphere *mySphere); 
float* getVertices(struct Sphere *mySphere); 
//const float* getNormals() const         { return normals.data(); }
//const float* getTexCoords() const       { return texCoords.data(); }
unsigned int* getIndices(struct Sphere *mySphere);
//const unsigned int* getLineIndices(struct Sphere *mySphere)   { return lineIndices.data(); }

void buildVertices(struct Sphere *mySphere);

void addVertex(struct Sphere *mySphere, float x, float y, float z);
void addIndices(struct Sphere *mySphere, unsigned int i1, unsigned int i2, unsigned int i3);
void buildInterleavedVertices(struct Sphere *mySphere);
unsigned int getIndexCount(struct Sphere *mySphere);
void computeFaceNormal( float *normal,  float x1, float y1, float z1,  // v1
                                        float x2, float y2, float z2,  // v2
                                        float x3, float y3, float z3);

void addTexCoord(struct Sphere *mySphere, float s, float t);
int getInterleavedStride(struct Sphere *mySphere);

float *getInterleavedVertices(struct Sphere *mySphere);

void clearArrays(struct Sphere *mySphere);


void addNormal(struct Sphere *mySphere, float x, float y, float z);
unsigned int getInterleavedVertexSize(struct Sphere *mySphere);
