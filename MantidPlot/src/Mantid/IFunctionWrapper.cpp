// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "IFunctionWrapper.h"
#include "MantidAPI/CompositeFunction.h"
#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/IPeakFunction.h"

void IFunctionWrapper::setFunction(const QString &name) {
  try {
    m_function = std::dynamic_pointer_cast<Mantid::API::CompositeFunction>(
        Mantid::API::FunctionFactory::Instance().createFunction(
            name.toStdString()));
    m_compositeFunction =
        std::dynamic_pointer_cast<Mantid::API::CompositeFunction>(m_function);
    m_peakFunction =
        std::dynamic_pointer_cast<Mantid::API::IPeakFunction>(m_function);
  } catch (...) {
    m_function.reset();
    m_compositeFunction.reset();
    m_peakFunction.reset();
  }
}

void IFunctionWrapper::setFunction(
    const std::shared_ptr<Mantid::API::IFunction> &function) {
  m_function = function;
  m_compositeFunction =
      std::dynamic_pointer_cast<Mantid::API::CompositeFunction>(m_function);
  m_peakFunction =
      std::dynamic_pointer_cast<Mantid::API::IPeakFunction>(m_function);
}
