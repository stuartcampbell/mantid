#ifndef MANTID_VISIBLEAXESCOLOR_H_
#define MANTID_VISIBLEAXESCOLOR_H_
#include "MantidVatesSimpleGuiViewWidgets/WidgetDllOption.h"
#include "pqRenderView.h"

namespace Mantid {
namespace Vates {
namespace SimpleGui {

/**
 *

  Copyright &copy; 2016 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
  National Laboratory & European Spallation Source

  This file is part of Mantid.

  Mantid is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Mantid is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  File change history is stored at: <https://github.com/mantidproject/mantid>
  Code Documentation is available at: <http://doxygen.mantidproject.org>
*/

class EXPORT_OPT_MANTIDVATES_SIMPLEGUI_VIEWWIDGETS VisibleAxesColor {
public:
  /**
   * Set the Rgb values for the color of the view's orientation axes Label.
   * @param useCurrentBackgroundColor Is this the initial loading or were the
   * views switched?
   * @param view The view which has its background color set.
   */
  void setOrientationAxesLabelColor(pqRenderView *view,
                                    bool useCurrentBackgroundColor);
  void setGridAxesColor(pqRenderView *view, bool /*on*/);
  void setScalarBarColor();

private:
};
}
}
}

#endif // MANTID_VISIBLEAXESCOLOR_H_
