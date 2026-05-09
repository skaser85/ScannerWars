#include "particles.h"

Particle *alloc_particle() {
  Particle *p = (Particle*)malloc(sizeof(Particle));
  memset(p, 0, sizeof(*p));
  p->living = true;
  return p;
}

Particles *alloc_particles() {
  Particles *p = (Particles*)malloc(sizeof(Particles));
  memset(p, 0, sizeof(*p));
  p->living = true;
  return p;
}

ParticlesCollection *alloc_particles_collection() {
  ParticlesCollection *p = (ParticlesCollection*)malloc(sizeof(ParticlesCollection));
  memset(p, 0, sizeof(*p));
  return p;
}

Particles *generate_particles(Vector2 start_pos, Colors colors) {

  Particles *particles = alloc_particles();
  size_t particle_amount = (size_t)get_random_non_zero_int(10, 30); 

  for (size_t i = 0; i < particle_amount; ++i) {
    Particle *p = alloc_particle();
    p->pos = (Vector2) { .x = start_pos.x, .y = start_pos.y };
    p->radius = (float)get_random_non_zero_int(5, 15);
    p->color = colors.items[GetRandomValue(0, colors.count)];
    p->velo = (Vector2) { .x = get_random_non_zero_int(-20, 20),
      .y = get_random_non_zero_int(-20, 20) };
    p->living = true;
    da_append(particles, *p);
  }

  return particles;
}

void update_particles(ParticlesCollection *particles) {
  da_foreach(Particles, p, particles) {
    if (p->living) {
      size_t dead_count = 0;
      da_foreach(Particle, it, p) {
        if (it->living) {
          if (
              (it->pos.x < 0 || it->pos.x > GetScreenWidth()) &&
              (it->pos.y < 0 || it->pos.y > GetScreenHeight()) 
             ) {
            it->living = false;
            dead_count++;
          } else {
            it->pos = Vector2Add(it->pos, it->velo);
          }
        }
      }
      if (dead_count >= p->count)
        p->living = false;
    }
  }
}


