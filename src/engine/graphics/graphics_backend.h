//
// Created by Sam Serrels on 30/11/2019.
//

#ifndef OPENLRR_GRAPHICS_BACKEND_H
#define OPENLRR_GRAPHICS_BACKEND_H

class GraphicsBackend {
public:
  virtual void startup() = 0;
  virtual void shutdown() = 0;
  virtual void drawFrame(double dt=0) = 0;
  virtual void resize() = 0;
};

#endif // OPENLRR_GRAPHICS_BACKEND_H
