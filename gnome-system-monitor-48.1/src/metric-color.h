#ifndef ENHANCED_METRIC_COLOR_H
#define ENHANCED_METRIC_COLOR_H

#include <gtk/gtk.h>
#include <initializer_list>
#include <optional>

namespace enhanced {

enum class MetricColor { neutral, accent, normal, elevated, warning };

inline void set_metric_color (GtkWidget *widget, MetricColor color) {
  const char *name = "metric-neutral";
  switch (color) {
    case MetricColor::neutral: break;
    case MetricColor::accent: name = "metric-accent"; break;
    case MetricColor::normal: name = "metric-normal"; break;
    case MetricColor::elevated: name = "metric-elevated"; break;
    case MetricColor::warning: name = "metric-warning"; break;
  }
  if (gtk_widget_has_css_class (widget, name)) return;
  for (const char *old : {"metric-neutral", "metric-accent", "metric-normal",
                          "metric-elevated", "metric-warning"})
    gtk_widget_remove_css_class (widget, old);
  gtk_widget_add_css_class (widget, name);
}

// UI warning thresholds, not manufacturer operating or safety limits.
inline MetricColor temperature_color (std::optional<double> celsius,
                                      double elevated, double warning) {
  if (!celsius) return MetricColor::neutral;
  if (*celsius >= warning) return MetricColor::warning;
  if (*celsius >= elevated) return MetricColor::elevated;
  return MetricColor::normal;
}

inline MetricColor pressure_color (std::optional<double> percent, bool gpu_load = false) {
  if (!percent) return MetricColor::neutral;
  if (*percent >= 90) return gpu_load ? MetricColor::accent : MetricColor::warning;
  if (*percent >= 70) return MetricColor::elevated;
  return MetricColor::normal;
}

inline MetricColor activity_color (bool available) {
  return available ? MetricColor::accent : MetricColor::neutral;
}

} // namespace enhanced
#endif
