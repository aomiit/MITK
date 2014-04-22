/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "org_mitk_gui_qt_surfaceregistration_Activator.h"

#include <QtPlugin>

#include "QmitkSurfaceRegistrationView.h"

namespace mitk {

void org_mitk_gui_qt_surfaceregistration_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(QmitkSurfaceRegistrationView, context)
}

void org_mitk_gui_qt_surfaceregistration_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}

}

Q_EXPORT_PLUGIN2(org_mitk_gui_qt_surfaceregistration, mitk::org_mitk_gui_qt_surfaceregistration_Activator)
