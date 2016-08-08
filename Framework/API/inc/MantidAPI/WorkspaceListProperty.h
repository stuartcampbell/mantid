#ifndef MANTID_API_WORKSPACELISTPROPERTY_H_
#define MANTID_API_WORKSPACELISTPROPERTY_H_

#include "MantidAPI/WorkspaceProperty.h"
#include "MantidKernel/DataItem.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <vector>

namespace Mantid {
namespace API {

/** WorkspaceListProperty : Property with value that constrains the contents to
  be a list of workspaces of a single type.

  @author Owen Arnold, ISIS, RAL
  @date 03/10/2013

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

template <typename TYPE = MatrixWorkspace>
class DLLExport WorkspaceListProperty
    : public Mantid::Kernel::PropertyWithValue<
          std::vector<boost::shared_ptr<TYPE>>> {
public:
  /// Typedef the value type of this property with value.
  typedef std::vector<boost::shared_ptr<TYPE>> WorkspaceListPropertyType;
  typedef Kernel::PropertyWithValue<WorkspaceListPropertyType> SuperClass;

  /**
  * WorkspaceListProperty
  * @param name: Name of the property
  * @param workspaces: Workspaces to hold
  * @param direction : Property directoin
  * @param optional : Optional or mandatory property
  * @param validator : Validator to use
  */
  explicit WorkspaceListProperty(
      const std::string &name, const WorkspaceListPropertyType workspaces,
      const unsigned int direction = Mantid::Kernel::Direction::Input,
      const PropertyMode::Type optional = PropertyMode::Mandatory,
      Mantid::Kernel::IValidator_sptr validator =
          Mantid::Kernel::IValidator_sptr(new Kernel::NullValidator))
      : Mantid::Kernel::PropertyWithValue<WorkspaceListPropertyType>(
            name, WorkspaceListPropertyType(0), validator, direction),
        m_optional(optional) {
    SuperClass::operator=(workspaces);
    auto errorMsg = isValid();
    if (!errorMsg.empty())
      throw std::invalid_argument(errorMsg);
  }

  /// Copy constructor, the default name stored in the new object is the same as
  /// the default name from the original object
  WorkspaceListProperty(const WorkspaceListProperty &right)
      : SuperClass(right), m_optional(right.m_optional) {}

  /**
  * Assignment overload
  * @param right : rhs workspace list property.
  */
  WorkspaceListProperty &operator=(const WorkspaceListProperty &right) {
    if (&right != this) {
      SuperClass::operator=(right);
      m_optional = right.m_optional;
    }
    return *this;
  }

  /**
  * Assignment overload
  * @param right : rhs workspace list property type.
  */
  WorkspaceListPropertyType &operator=(const WorkspaceListPropertyType &right) {
    return SuperClass::operator=(right);
  }

  /**
  * Clone operation.
  */
  WorkspaceListProperty<TYPE> *clone() const {
    return new WorkspaceListProperty<TYPE>(*this);
  }

  //const WorkspaceListPropertyType  &operator() const { return SuperClass::operator()(); }
  const WorkspaceListPropertyType &list() const { return SuperClass::operator()(); }

  /** Get the name of the workspace
  *  @return The workspace's name
  */
  std::string value() const override { return ""; }

  /** Get the value the property was initialised with -its default value
  *  @return The default value
  */
  std::string getDefault() const override { return ""; }

  /// Is the input workspace property optional?
  bool isOptional() const {
    return m_optional == PropertyMode::Optional;
  }

  std::string isValid() const override {
    std::string error;
    auto workspaces = SuperClass::m_value;

    for (auto &wksp : workspaces) {
      auto group = boost::dynamic_pointer_cast<WorkspaceGroup>(wksp);

      if (group) {
        auto names = group->getNames();
        if (std::any_of(names.cbegin(), names.cend(),
                        [](const std::string &s) { return s.size() > 0; })) {
          error = "WorkspaceGroups which exist in the ADS are not allowed.";
          break;
        }
      }
    }

    return error + SuperClass::isValid();
  }

  std::string
  setDataItems(const std::vector<boost::shared_ptr<Kernel::DataItem>> &items) override {
    std::string error;

    std::vector<boost::shared_ptr<TYPE>> tmp(items.size());

    for (int i = 0; i < items.size(); i++)
      tmp[i] = boost::dynamic_pointer_cast<TYPE>(items[i]);

    auto valid =
        !std::any_of(tmp.cbegin(), tmp.cend(),
                     [](boost::shared_ptr<TYPE> x) { return x == nullptr; });

    if (valid) {
      SuperClass::m_value = tmp;
    } else {
      clear();
      error = "Attempted to add one of more invalid types.";
    }

    return error + isValid();
  }

  /**
  * Destructor
  */
  virtual ~WorkspaceListProperty() {}

private:
  void clear() { SuperClass::operator=(WorkspaceListPropertyType(0)); }

  /// Flag indicating whether the type is optional or not.
  PropertyMode::Type m_optional;
};

} // namespace API
} // namespace Mantid

#endif /* MANTID_API_WORKSPACELISTPROPERTY_H_ */