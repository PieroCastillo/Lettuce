#include <print>
#include <vector>

#include <glm/glm.hpp>

constexpr uint32_t maxMeshVertices = 256;
constexpr uint32_t maxMeshTriangles = 256;

/*
main bezier: pc0, pc1, pc2, pc3 depends of alpha, alpha : vec3, alpha says how much bezier is curved, and its direction
leaves bezier: p2, p3 depends of beta: beta:vec3, beta same as alpha, but for secondary bezier

get pc0, pc1, pc2, pc3 (Control points)
calculate p+ & p- (by angle alpha & width w)

create bezier curve with k points (min LOD = 1 "two points", max LOD = k)
subcurves with w points
(k,w) dependen de LOD
step = 1/(k-1)
t = step
p = p0
pp = p+
ppp = p-
for(idx=0;idx<k;++k)
{
    v = bezier(pc0, pc1, pc2, pc3, t)
    v1 = cuadrilatero(v, p, pp)
    v2 = cuadrilatero(v, p, ppp)
    vertices.add(v)
    vertices.add(v1)
    vertices.add(v2)
    triangles.add(v, p, pp)
    triangles.add(v, pp, v1)
    triangles.add(v, ppp, p)
    triangles.add(v, v2, ppp)
    p = v
    pp = v1
    ppp = v2

    t+= step
}

mode 1: none
    finish

mode 2: interleaved
    cada u pasos
        control points: p0, p1, p2
        p : vertices que pertence a la curva bezier principal
            en interleaved, pp "nace" de p+, en la siguiente iteracion nace de p-
        pp : vertice "paralelo" a p0, obtener de vertices[]
        t = step
        p = p0
        for(idx=0; idx<k/2; idx++)
        {
            v = bezier(pc0, pc1, pc2, t)
            v1 = cuadrilatero(v, p, pp)
            vertices.add(v)
            vertices.add(v1)
            triangles.add(v, p,  pp)
            triangles.add(v, pp, v1)
            p = v
            t+= step
        }

mode 3: opposite
        cada u pasos
        control points: p0, p1, p2
        p : vertices que pertence a la curva bezier principal
            en opposite:
        pp+ : vertice "paralelo" a p, obtener de vertices[], "nace" de p+
        pp- : vertice "paralelo" a p, obtener de vertices[], "nace" de p-
        t = step
        p = p0
        for(idx=0; idx<k/2; idx++)
        {
            v = bezier(pc0, pc1, pc2, t)
            v1 = cuadrilatero(v, p, pp+)
            vertices.add(v)
            vertices.add(v1)
            triangles.add(v, p,  pp+)
            triangles.add(v, pp+, v1)
            
            w = v
            w1 = cuadrilatero(w, p, pp-)
            vertices.add(w)
            vertices.add(w1)
            triangles.add(w, p,  pp-)
            triangles.add(w, pp-, w1)
            p = v

            t+= step
        }
*/
void meshMain(const glm::vec3 SV_DispatchThreadID,
              const glm::vec3 SV_GroupThreadID,
              std::vector<glm::vec3>& triangles, // indices
              std::vector<glm::vec3>& vertices)  // positions
{

}

int main()
{
    uint32_t groupXCount, groupYCount, groupZCount;
    uint32_t localXcount, localYCount, localZCount;

    


    return 0;
}