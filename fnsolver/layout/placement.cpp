#include <fnsolver/layout/placement.h>

#include <fnsolver/data/fnsite.h>
#include <fnsolver/data/probe.h>

Placement::Placement(const FnSite &site, const Probe &probe)
    : site(&site), probe(&probe) {}

const FnSite &Placement::get_site() const {
  return *site;
}

const Probe &Placement::get_probe() const {
  return *probe;
}
