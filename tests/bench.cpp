/* Copyright (C) 2003 Massachusetts Institute of Technology  
%
%  This program is free software; you can redistribute it and/or modify
%  it under the terms of the GNU General Public License as published by
%  the Free Software Foundation; either version 2, or (at your option)
%  any later version.
%
%  This program is distributed in the hope that it will be useful,
%  but WITHOUT ANY WARRANTY; without even the implied warranty of
%  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  GNU General Public License for more details.
%
%  You should have received a copy of the GNU General Public License
%  along with this program; if not, write to the Free Software Foundation,
%  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dactyl.h"

double one(const vec &) { return 1.0; }
static double width = 20.0;
double bump(const vec &v) { return (fabs(v.z()-50.0) > width)?1.0:12.0; }

struct bench {
  double time; // In seconds.
  double gridsteps;
};

bench bench_periodic(const double rmax, const double zmax,
                     double eps(const vec &)) {
  const double a = 10.0;
  const double gridpts = (zmax==0.0)?a*rmax:a*a*rmax*zmax;
  const double ttot = 5.0 + 1e5/gridpts;
  const int m = 0;

  volume v = volcyl(rmax,zmax,a);
  mat ma(v, eps);
  fields f(&ma, m);
  f.use_bloch(0.0);
  f.add_point_source(Ep, 0.7, 2.5, 0.0, 4.0, vec(0.5, 0.4), 1.0);
  f.add_point_source(Ez, 0.8, 0.6, 0.0, 4.0, vec(0.401, 0.301), 1.0);

  clock_t start = clock();
  while (f.time() < ttot) f.step();
  bench b;
  b.time = (clock()-start)*(1.0/CLOCKS_PER_SEC);
  b.gridsteps = ttot*a*2*gridpts;
  //f.print_times();
  return b;
}

bench bench_flux_1d(const double zmax,
                    double eps(const vec &)) {
  const double a = 10.0;
  const double gridpts = a*zmax;
  const double ttot = 10.0 + 1e5/zmax;

  volume v = volone(zmax,a);
  mat ma(v, eps);
  ma.use_pml_everywhere(zmax/6);

  fields f(&ma);
  f.use_real_fields();
  f.add_point_source(Ex, 0.7, 2.5, 0.0, 3.0, vec(zmax/2+0.3), 1.0);
  flux_plane *left = f.add_flux_plane(vec(zmax/3.0), vec(zmax/3.0));
  flux_plane *right = f.add_flux_plane(vec(zmax*2.0/3.0), vec(zmax*2.0/3.0));

  while (f.time() <= f.find_last_source()) f.step();

  volume mid = volone(zmax/3,a);
  mid.origin = vec(zmax/3);
  double flux_energy=0.0;
  clock_t start = clock();
  while (f.time() < ttot) {
    f.step();
    flux_energy += (c/a)*(right->flux() - left->flux());
  }
  bench b;
  b.time = (clock()-start)*(1.0/CLOCKS_PER_SEC);
  b.gridsteps = ttot*a*2*gridpts;
  //f.print_times();
  return b;
}

bench bench_flux_1d_old(const double zmax,
                        double eps(const vec &)) {
  const double a = 10.0;
  const double gridpts = a*zmax;
  const double ttot = 10.0 + 1e5/zmax;

  volume v = volone(zmax,a);
  mat ma(v, eps);
  ma.use_pml_everywhere(zmax/6);

  fields f(&ma);
  f.use_real_fields();
  f.add_point_source(Ex, 0.7, 2.5, 0.0, 3.0, vec(zmax/2+0.3), 1.0);
  flux_plane *left = f.add_flux_plane(vec(zmax/3.0), vec(zmax/3.0));
  flux_plane *right = f.add_flux_plane(vec(zmax*2.0/3.0), vec(zmax*2.0/3.0));

  while (f.time() <= f.find_last_source()) f.step_old();

  volume mid = volone(zmax/3,a);
  mid.origin = vec(zmax/3);
  double flux_energy=0.0;
  clock_t start = clock();
  while (f.time() < ttot) {
    f.step_old();
    flux_energy += (c/a)*(right->flux() - left->flux());
  }
  bench b;
  b.time = (clock()-start)*(1.0/CLOCKS_PER_SEC);
  b.gridsteps = ttot*a*2*gridpts;
  //f.print_times();
  return b;
}

bench bench_2d(const double xmax, const double ymax,
               double eps(const vec &)) {
  const double a = 10.0;
  const double gridpts = a*a*xmax*ymax;
  const double ttot = 5.0 + 1e5/gridpts;

  volume v = voltwo(xmax,ymax,a);
  mat ma(v, eps);
  fields f(&ma);
  f.use_metal_everywhere();
  f.add_point_source(Ez, 0.8, 0.6, 0.0, 4.0, vec2d(0.401, 0.301));
  f.add_point_source(Hz, 0.8, 0.7, 0.0, 4.0, vec2d(0.431, 0.2));

  while (f.time() < f.find_last_source()) f.step();
  const double tend = f.time() + ttot;
  clock_t start = clock();
  while (f.time() < tend) f.step();
  bench b;
  b.time = (clock()-start)*(1.0/CLOCKS_PER_SEC);
  b.gridsteps = ttot*a*2*gridpts;
  //f.print_times();
  return b;
}

const double te_tm_2d_time = 2e5;

bench bench_2d_tm(const double xmax, const double ymax,
               double eps(const vec &)) {
  const double a = 10.0;
  const double gridpts = a*a*xmax*ymax;
  const double ttot = 5.0 + te_tm_2d_time/gridpts;

  volume v = voltwo(xmax,ymax,a);
  mat ma(v, eps);
  fields f(&ma);
  f.use_metal_everywhere();
  f.add_point_source(Ez, 0.8, 0.6, 0.0, 4.0, vec2d(0.401, 0.301));

  while (f.time() < f.find_last_source()) f.step();
  const double tend = f.time() + ttot;
  clock_t start = clock();
  while (f.time() < tend) f.step();
  bench b;
  b.time = (clock()-start)*(1.0/CLOCKS_PER_SEC);
  b.gridsteps = ttot*a*2*gridpts;
  //f.print_times();
  return b;
}

bench bench_2d_tm_old(const double xmax, const double ymax,
               double eps(const vec &)) {
  const double a = 10.0;
  const double gridpts = a*a*xmax*ymax;
  const double ttot = 5.0 + te_tm_2d_time/gridpts;

  volume v = voltwo(xmax,ymax,a);
  mat ma(v, eps);
  fields f(&ma);
  f.use_metal_everywhere();
  f.add_point_source(Ez, 0.8, 0.6, 0.0, 4.0, vec2d(0.401, 0.301));

  while (f.time() < f.find_last_source()) f.step_old();
  const double tend = f.time() + ttot;
  clock_t start = clock();
  while (f.time() < tend) f.step_old();
  bench b;
  b.time = (clock()-start)*(1.0/CLOCKS_PER_SEC);
  b.gridsteps = ttot*a*2*gridpts;
  //f.print_times();
  return b;
}

bench bench_2d_te(const double xmax, const double ymax,
               double eps(const vec &)) {
  const double a = 10.0;
  const double gridpts = a*a*xmax*ymax;
  const double ttot = 5.0 + te_tm_2d_time/gridpts;

  volume v = voltwo(xmax,ymax,a);
  mat ma(v, eps);
  fields f(&ma);
  f.use_metal_everywhere();
  f.add_point_source(Ex, 0.8, 0.6, 0.0, 4.0, vec2d(0.401, 0.301));
  f.add_point_source(Hz, 0.6, 0.6, 0.0, 4.0, vec2d(0.7, 0.5));

  while (f.time() < f.find_last_source()) f.step();
  const double tend = f.time() + ttot;
  clock_t start = clock();
  while (f.time() < tend) f.step();
  bench b;
  b.time = (clock()-start)*(1.0/CLOCKS_PER_SEC);
  b.gridsteps = ttot*a*2*gridpts;
  //f.print_times();
  return b;
}

bench bench_2d_te_old(const double xmax, const double ymax,
               double eps(const vec &)) {
  const double a = 10.0;
  const double gridpts = a*a*xmax*ymax;
  const double ttot = 5.0 + te_tm_2d_time/gridpts;

  volume v = voltwo(xmax,ymax,a);
  mat ma(v, eps);
  fields f(&ma);
  f.use_metal_everywhere();
  f.add_point_source(Ex, 0.8, 0.6, 0.0, 4.0, vec2d(0.401, 0.301));
  f.add_point_source(Hz, 0.6, 0.6, 0.0, 4.0, vec2d(0.7, 0.5));

  while (f.time() < f.find_last_source()) f.step_old();
  const double tend = f.time() + ttot;
  clock_t start = clock();
  while (f.time() < tend) f.step_old();
  bench b;
  b.time = (clock()-start)*(1.0/CLOCKS_PER_SEC);
  b.gridsteps = ttot*a*2*gridpts;
  //f.print_times();
  return b;
}

#define showbench(name, bb) \
  master_printf("%s\n", name); \
  { \
    bench b = bb; \
    master_printf("  total time:    \t%lg s\n  normalized time:\t%lg s/Mgs\n", \
                  b.time, b.time*1e6/b.gridsteps); \
  }

int main(int argc, char **argv) {
  initialize mpi(argc, argv);
  master_printf("Benchmarking...\n");

  showbench("Periodic 6x4 ", bench_periodic(6.0, 4.0, one));
  showbench("Periodic 12x1", bench_periodic(12.0, 1.0, one));
  showbench("Periodic 1x12", bench_periodic(1.0, 12.0, one));
  showbench("Periodic 12x0", bench_periodic(12.0, 0.0, one));
  showbench("Periodic 12x12", bench_periodic(12.0, 12.0, one));

  width = 20.0;
  showbench("Flux 1D 100", bench_flux_1d(100.0, bump));
  width = 10.0;
  showbench("Flux 1D 100", bench_flux_1d(100.0, bump));
  width = 300.0;
  showbench("Flux 1D 100", bench_flux_1d(100.0, bump));
  width = 300.0;
  showbench("Flux 1D 100 old", bench_flux_1d_old(100.0, bump));

  showbench("2D 6x4 ", bench_2d(6.0, 4.0, one));
  showbench("2D 12x12 ", bench_2d(12.0, 12.0, one));
  showbench("2D 12x12 ", bench_2d(12.0, 12.0, one));

  showbench("2D TM 6x4 ", bench_2d_tm(6.0, 4.0, one));
  showbench("2D TM 12x12 ", bench_2d_tm(12.0, 12.0, one));
  showbench("2D TM 12x12 old ", bench_2d_tm_old(12.0, 12.0, one));

  showbench("2D TE 2x2 ", bench_2d_te(2.0, 2.0, one));
  showbench("2D TE 2x2 old ", bench_2d_te_old(2.0, 2.0, one));
  showbench("2D TE 10x11 ", bench_2d_te(10.0, 11.0, one));
  showbench("2D TE 10x11 old ", bench_2d_te_old(10.0, 11.0, one));

  master_printf("\nnote: 1 Mgs = 1 million grid point time steps\n");

  exit(0);
}

