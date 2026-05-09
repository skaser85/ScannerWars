#ifndef PARTICLES_H
#define PARTICLES_H

#include "utils.h"

#include "raylib.h"
#include "raymath.h"

#include "nob.h"

typedef struct {
  Vector2 pos;
  float radius;
  Color color;
  Vector2 velo;
  bool living;
} Particle;

typedef struct {
  Particle *items;
  size_t count;
  size_t capacity;
  bool living;
} Particles;

typedef struct {
  Particles *items;
  size_t count; 
  size_t capacity;
} ParticlesCollection;

Particle *alloc_particle();
Particles *alloc_particles();
ParticlesCollection *alloc_particles_collection();
Particles *generate_particles(Vector2 start_pos, Colors colors);
void update_particles(ParticlesCollection *particles);

#endif // PARTICLES_H
