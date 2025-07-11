#ifndef FNSOLVER_LAYOUT_PLACEMENT_H
#define FNSOLVER_LAYOUT_PLACEMENT_H

#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/probe.h>

class Placement {
  public:
    Placement(const FnSite &site, const Probe &probe);

    Placement(const Placement &other) = default;
    Placement(Placement &&other) = default;
    Placement &operator=(const Placement &other) = default;
    Placement &operator=(Placement &&other) = default;

    const FnSite &get_site() const;
    const Probe &get_probe() const;
  private:
    const FnSite *site;
    const Probe *probe;
};

#endif // FNSOLVER_LAYOUT_PLACEMENT_H

