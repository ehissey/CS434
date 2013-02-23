

for all vertices v
    project v  -----> VERTEX_SHADER(v)
for all triangles t
    clip t
    compute AABB aabb of projected t
    rasterization setup for t
    GEOMETRY_SHADER(t)
    for all rows r in aabb
        for all pixels p in r
            if p outside projected t
              continue
            if !zbuffer(p, t)
                continue
            p.color = ComputeColor(rpvs at p) ------> FRAGMENT_SHADER(rpvs at p)


ReflectionFragmentShader(normal, eye, surfacePoint)
  r = reflect(eye-surfacePoint, normal)
  if intersect(r, depth image of diffuse bunny)
    return intersection color
  if intersect(r, base quad billboard)
    return intersectin color
  return EnvironmentMap(r)
  
