#include "Sphere.h"
#include "vec.h"
#include <math.h>
#include <stdio.h>

/* Just did the flat shading for buildVertices()...just easier.*/

// The Sphere structure is defined in Sphere.h

// Creates a sphere and returns a pointer to it.
struct Sphere * createSphere(float radius, int sectors, int stacks) {
    printf("Hi\n");
    // malloc is in stdlib.h, which is in vec.h which is here.
    struct Sphere *mySphere = malloc(sizeof(struct Sphere));
    printf("Created struct object.\n");
    // printSelf(&mySphere);
    
    mySphere->radius = radius;
    printf("Assigned radius.\n");
    mySphere->sectors = sectors;
    printf("Assigned sectors;\n");
    mySphere->stacks = stacks;
    printf("Getting old, but assigned stacks.\n");
    mySphere->interleavedStride = 32;
    mySphere->vertices = vector_create();
    mySphere->interleavedVertices = vector_create();
    mySphere->normals= vector_create();
    mySphere->texCoords = vector_create();
    mySphere->indices = vector_create();
    mySphere->lineIndices = vector_create();
    printf("Building da vertices\n");
    buildVertices(mySphere);
    printf("Celebwashun time... :D\n");
    return mySphere;
}

void printSelf(struct Sphere *mySphere) 
{
    printf("===== Sphere =====\n");
    float f = mySphere->radius;
    printf("Hi");
    printf("        Radius: %f\n", mySphere->radius);
    printf("  Sector Count: %d\n", mySphere->sectors);
    printf("   Stack Count: %d\n", mySphere->stacks);
    //printf("Smooth Shading: " << (smooth ? "true" : "false") << "\n"
    printf("Triangle Count: %d\n", getTriangleCount(mySphere));
    printf("   Index Count: %d\n", getIndexCount(mySphere));
    printf("  Vertex Count: %d\n", getVertexCount(mySphere)); 
    printf("  Normal Count: %d\n", getNormalCount(mySphere)); 
    printf("TexCoord Count: %d\n", getTexCoordCount(mySphere));
}

unsigned int getVertexCount(struct Sphere *mySphere) { return (unsigned int)vector_size(mySphere->vertices) / 3; }
unsigned int getNormalCount(struct Sphere *mySphere) { return (unsigned int)vector_size(mySphere->normals) / 3; }
unsigned int getTexCoordCount(struct Sphere *mySphere) { return (unsigned int)vector_size(mySphere->texCoords) / 2; }
unsigned int getIndexCount(struct Sphere *mySphere) { return vector_size(mySphere->indices); }
unsigned int getLineIndexCount(struct Sphere *mySphere) { return (unsigned int)vector_size(mySphere->lineIndices); }
unsigned int getTriangleCount(struct Sphere *mySphere) { return getIndexCount(mySphere) / 3; }
unsigned int getVertexSize(struct Sphere *mySphere) { return (unsigned int)vector_size(mySphere->vertices) * sizeof(float); }
unsigned int getNormalSize(struct Sphere *mySphere) { return (unsigned int)vector_size(mySphere->normals) * sizeof(float); }
unsigned int getTexCoordSize(struct Sphere *mySphere) { return (unsigned int)vector_size(mySphere->texCoords) * sizeof(float); }
unsigned int getIndexSize(struct Sphere *mySphere) { return (unsigned int)vector_size(mySphere->indices) * sizeof(unsigned int); }
unsigned int getLineIndexSize(struct Sphere *mySphere) { return (unsigned int)vector_size(mySphere->lineIndices) * sizeof(unsigned int); } 
float* getVertices(struct Sphere *mySphere) { return mySphere->vertices; }
//const float* getNormals() const         { return normals.data(); }
//const float* getTexCoords() const       { return texCoords.data(); }
unsigned int* getIndices(struct Sphere *mySphere) { return mySphere->indices; }
//const unsigned int* getLineIndices(struct Sphere *mySphere)   { return lineIndices.data(); }

void buildVertices(struct Sphere *mySphere) 
{
    const float PI = acos(-1);

    // tmp vertex definition (x,y,z,s,t)
    struct Vertex
    {
        float x, y, z, s, t;
    };
    struct Vertex *tmpVertices = vector_create();
    // std::vector<Vertex> tmpVertices;

    float sectorStep = 2 * PI / mySphere->sectors;
    float stackStep = PI / mySphere->stacks;
    float sectorAngle, stackAngle;

    // compute all vertices first, each vertex contains (x,y,z,s,t) except normal
    for(int i = 0; i <= mySphere->stacks; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        float xy = mySphere->radius * cosf(stackAngle);       // r * cos(u)
        float z = mySphere->radius * sinf(stackAngle);        // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= mySphere->sectors; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi
            
            struct Vertex vertex;
            vertex.x = xy * cosf(sectorAngle);      // x = r * cos(u) * cos(v)
            vertex.y = xy * sinf(sectorAngle);      // y = r * cos(u) * sin(v)
            vertex.z = z;                           // z = r * sin(u)
            vertex.s = (float)j/mySphere->sectors;        // s
            vertex.t = (float)i/mySphere->stacks;         // t
            vector_add(&tmpVertices, vertex);
            //tmpVertices.push_back(vertex);
        }
    }

    // clear memory of prev arrays
    clearArrays(mySphere);

    struct Vertex v1, v2, v3, v4;                          // 4 vertex positions and tex coords
    //std::vector<float> n;                           // 1 face normal
    float *n;

    int i, j, k, vi1, vi2;
    int index = 0;                                  // index for vertex
    for(i = 0; i < mySphere->stacks; ++i)
    {
        vi1 = i * (mySphere->sectors + 1);                // index of tmpVertices
        vi2 = (i + 1) * (mySphere->sectors + 1);

        for(j = 0; j < mySphere->sectors; ++j, ++vi1, ++vi2)
        {
            // get 4 vertices per sector
            //  v1--v3
            //  |    |
            //  v2--v4
            v1 = tmpVertices[vi1];
            v2 = tmpVertices[vi2];
            v3 = tmpVertices[vi1 + 1];
            v4 = tmpVertices[vi2 + 1];

            // if 1st stack and last stack, store only 1 triangle per sector
            // otherwise, store 2 triangles (quad) per sector
            if(i == 0) // a triangle for first stack ==========================
            {
                // put a triangle
                addVertex(mySphere, v1.x, v1.y, v1.z);
                addVertex(mySphere, v2.x, v2.y, v2.z);
                addVertex(mySphere, v4.x, v4.y, v4.z);

                // put tex coords of triangle
                addTexCoord(mySphere, v1.s, v1.t);
                addTexCoord(mySphere, v2.s, v2.t);
                addTexCoord(mySphere, v4.s, v4.t);

                // put normal
                computeFaceNormal(n, v1.x,v1.y,v1.z, v2.x,v2.y,v2.z, v4.x,v4.y,v4.z);
                for(k = 0; k < 3; ++k)  // same normals for 3 vertices
                {
                    addNormal(mySphere, n[0], n[1], n[2]);
                }

                // put indices of 1 triangle
                addIndices(mySphere, index, index+1, index+2);

                // indices for line (first stack requires only vertical line)
                vector_add(&(mySphere->lineIndices), index);
                //lineIndices.push_back(index);
                 vector_add(&(mySphere->lineIndices), index+1);
                //lineIndices.push_back(index+1);

                index += 3;     // for next
            }
            else if(i == ((mySphere->stacks)-1)) // a triangle for last stack =========
            {
                // put a triangle
                addVertex(mySphere, v1.x, v1.y, v1.z);
                addVertex(mySphere, v2.x, v2.y, v2.z);
                addVertex(mySphere, v3.x, v3.y, v3.z);

                // put tex coords of triangle
                addTexCoord(mySphere, v1.s, v1.t);
                addTexCoord(mySphere, v2.s, v2.t);
                addTexCoord(mySphere, v3.s, v3.t);

                // put normal
                computeFaceNormal(n, v1.x,v1.y,v1.z, v2.x,v2.y,v2.z, v3.x,v3.y,v3.z);
                for(k = 0; k < 3; ++k)  // same normals for 3 vertices
                {
                    addNormal(mySphere, n[0], n[1], n[2]);
                }

                // put indices of 1 triangle
                addIndices(mySphere, index, index+1, index+2);

                // indices for lines (last stack requires both vert/hori lines)
                vector_add(&(mySphere->lineIndices), index);
                // lineIndices.push_back(index);
                vector_add(&(mySphere->lineIndices), index+1);
                //lineIndices.push_back(index+1);
                vector_add(&(mySphere->lineIndices), index);
                //lineIndices.push_back(index);
                vector_add(&(mySphere->lineIndices), index+2);
                //lineIndices.push_back(index+2);

                index += 3;     // for next
            }
            else // 2 triangles for others ====================================
            {
                // put quad vertices: v1-v2-v3-v4
                addVertex(mySphere, v1.x, v1.y, v1.z);
                addVertex(mySphere, v2.x, v2.y, v2.z);
                addVertex(mySphere, v3.x, v3.y, v3.z);
                addVertex(mySphere, v4.x, v4.y, v4.z);

                // put tex coords of quad
                addTexCoord(mySphere, v1.s, v1.t);
                addTexCoord(mySphere, v2.s, v2.t);
                addTexCoord(mySphere, v3.s, v3.t);
                addTexCoord(mySphere, v4.s, v4.t);

                // put normal
                computeFaceNormal(n, v1.x,v1.y,v1.z, v2.x,v2.y,v2.z, v3.x,v3.y,v3.z);
                for(k = 0; k < 4; ++k)  // same normals for 4 vertices
                {
                    addNormal(mySphere, n[0], n[1], n[2]);
                }

                // put indices of quad (2 triangles)
                addIndices(mySphere, index, index+1, index+2);
                addIndices(mySphere, index+2, index+1, index+3);

                // indices for lines
                vector_add(&(mySphere->lineIndices), index);
                vector_add(&(mySphere->lineIndices), index+1);
                vector_add(&(mySphere->lineIndices), index);
                vector_add(&(mySphere->lineIndices), index+2);

                index += 4;     // for next
            }
        }
    }

    // generate interleaved vertex array as well
    buildInterleavedVertices(mySphere);
}

void addVertex(struct Sphere *mySphere, float x, float y, float z)
{
    vector_add(&(mySphere->vertices), x);
    vector_add(&(mySphere->vertices), y);
    vector_add(&(mySphere->vertices), z);
}

void addIndices(struct Sphere *mySphere, unsigned int i1, unsigned int i2, unsigned int i3)
{
    vector_add(&(mySphere->indices), i1);
    vector_add(&(mySphere->indices), i2);
    vector_add(&(mySphere->indices), i3);
}

void buildInterleavedVertices(struct Sphere *mySphere)
{
    mySphere->interleavedVertices = vector_create();

    size_t i, j;    
    size_t count =  vector_size(mySphere->vertices);

    for(i = 0, j = 0; i < count; i += 3, j += 2)
    {
        vector_add(&(mySphere->interleavedVertices), mySphere->vertices[i]);
        vector_add(&(mySphere->interleavedVertices), mySphere->vertices[i+1]);
        vector_add(&(mySphere->interleavedVertices), mySphere->vertices[i+2]);

        vector_add(&(mySphere->interleavedVertices), mySphere->normals[i]);
        vector_add(&(mySphere->interleavedVertices), mySphere->normals[i+1]);
        vector_add(&(mySphere->interleavedVertices), mySphere->normals[i+2]);

        vector_add(&(mySphere->interleavedVertices), mySphere->texCoords[j]);
        vector_add(&(mySphere->interleavedVertices), mySphere->texCoords[j+1]);
    }
}

///////////////////////////////////////////////////////////////////////////////
// return face normal of a triangle v1-v2-v3
// if a triangle has no surface (normal length = 0), then return a zero vector
///////////////////////////////////////////////////////////////////////////////
void computeFaceNormal( float *normal,  float x1, float y1, float z1,  // v1
                                        float x2, float y2, float z2,  // v2
                                        float x3, float y3, float z3)  // v3
{
    const float EPSILON = 0.000001f;

    vector_add(&normal, 0.0f);
    vector_add(&normal, 0.0f);
    vector_add(&normal, 0.0f);
    // std::vector<float> normal(3, 0.0f);     // default return value (0,0,0)
    float nx, ny, nz;

    // find 2 edge vectors: v1-v2, v1-v3
    float ex1 = x2 - x1;
    float ey1 = y2 - y1;
    float ez1 = z2 - z1;
    float ex2 = x3 - x1;
    float ey2 = y3 - y1;
    float ez2 = z3 - z1;

    // cross product: e1 x e2
    nx = ey1 * ez2 - ez1 * ey2;
    ny = ez1 * ex2 - ex1 * ez2;
    nz = ex1 * ey2 - ey1 * ex2;

    // normalize only if the length is > 0
    
    float length = sqrtf(nx * nx + ny * ny + nz * nz);
    if (length > EPSILON)
    {
        // normalize
        float lengthInv = 1.0f / length;
        normal[0] = nx * lengthInv;
        normal[1] = ny * lengthInv;
        normal[2] = nz * lengthInv;
    }
}

void addTexCoord(struct Sphere *mySphere, float s, float t)
{
    vector_add(&(mySphere->texCoords), s);
    vector_add(&(mySphere->texCoords), t);
}

int getInterleavedStride(struct Sphere *mySphere) { return mySphere->interleavedStride; }   // should be 32 bytes

float *getInterleavedVertices(struct Sphere *mySphere) 
{
    return mySphere->interleavedVertices;
}

void clearArrays(struct Sphere *mySphere) 
{
    // In the original Sphere.cpp, they used vector::swap() which swaps two vectors,
    // so they combined the vector constructor (to make a new vector) and swapped it
    // with the old--in this case, I'm just allocating new memory. That's probably
    // really bad actually because i don't know about the old memory, but i think this is
    // only called in one place so it shouldn't be too terrible.
    mySphere->vertices = vector_create();
    mySphere->normals = vector_create();
    mySphere->texCoords = vector_create();
    mySphere->indices = vector_create();
    mySphere->lineIndices = vector_create();
}

void addNormal(struct Sphere *mySphere, float x, float y, float z) 
{
    vector_add(&(mySphere->normals), x);
    vector_add(&(mySphere->normals), y);
    vector_add(&(mySphere->normals), z);
}

unsigned int getInterleavedVertexSize(struct Sphere *mySphere) 
{ return (unsigned int)vector_size(mySphere->interleavedVertices) * sizeof(float); }