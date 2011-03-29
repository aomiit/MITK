/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkMITKIGTTrackingToolboxView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>
#include <qfiledialog.h>

// MITK
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkTrackingDeviceSourceConfigurator.h>



const std::string QmitkMITKIGTTrackingToolboxView::VIEW_ID = "org.mitk.views.mitkigttrackingtoolbox";

QmitkMITKIGTTrackingToolboxView::QmitkMITKIGTTrackingToolboxView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
  m_TrackingTimer = new QTimer(this);	
  m_tracking = false;
}

QmitkMITKIGTTrackingToolboxView::~QmitkMITKIGTTrackingToolboxView()
{
  
}


void QmitkMITKIGTTrackingToolboxView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkMITKIGTTrackingToolboxViewControls;
    m_Controls->setupUi( parent );
 
    connect( m_Controls->m_LoadTools, SIGNAL(clicked()), this, SLOT(OnLoadTools()) );
    connect( m_Controls->m_StartTracking, SIGNAL(clicked()), this, SLOT(OnStartTracking()) );
    connect( m_Controls->m_StopTracking, SIGNAL(clicked()), this, SLOT(OnStopTracking()) );
    connect( m_TrackingTimer, SIGNAL(timeout()), this, SLOT(UpdateTrackingTimer()));
    connect( m_Controls->m_EnableLogging, SIGNAL(clicked()), this, SLOT(OnEnableLoggingClicked()));
    connect( m_Controls->m_ChooseFile, SIGNAL(clicked()), this, SLOT(OnChooseFileClicked()));

    this->m_Controls->m_configurationWidget->EnableAdvancedUserControl(false);
  }
}


void QmitkMITKIGTTrackingToolboxView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkMITKIGTTrackingToolboxView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkMITKIGTTrackingToolboxView::OnLoadTools()
{
  //read in filename
  QString filename = QFileDialog::getOpenFileName(NULL,tr("Open Toolfile"), "/", tr("All Files (*.*)")); //later perhaps: tr("Toolfile (*.tfl)"
  if (filename.isNull()) return;
  
  //initialize tool storage
  m_toolStorage = mitk::NavigationToolStorage::New();
  
  //read tool storage from disk
  mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(GetDataStorage());
  m_toolStorage = myDeserializer->Deserialize(filename.toStdString());
  if (m_toolStorage.IsNull()) 
	{
	MessageBox(myDeserializer->GetErrorMessage());
	m_toolStorage = NULL;
	return;
  }

  //update label
  QString toolLabel = QString("Loaded Tools: ") + QString::number(m_toolStorage->GetToolCount()) + " Tools from " + filename;
  m_Controls->m_toolLabel->setText(toolLabel);
}

void QmitkMITKIGTTrackingToolboxView::OnStartTracking()
{
//check if everything is ready to start tracking
if (this->m_toolStorage.IsNull())
  {
  MessageBox("Error: No Tools Loaded Yet!");
  return;
  }
else if (this->m_toolStorage->GetToolCount() == 0)
  {
  MessageBox("Error: No Way To Track Without Tools!");
  return;
  }

//set configuration finished
this->m_Controls->m_configurationWidget->ConfigurationFinished();

//build the IGT pipeline
mitk::TrackingDeviceSourceConfigurator::Pointer myTrackingDeviceSourceFactory = mitk::TrackingDeviceSourceConfigurator::New(this->m_toolStorage,this->m_Controls->m_configurationWidget->GetTrackingDevice());
m_TrackingDeviceSource = myTrackingDeviceSourceFactory->CreateTrackingDeviceSource(this->m_ToolVisualizationFilter);
m_TrackingDeviceSource->Connect();
m_TrackingDeviceSource->StartTracking();
m_TrackingTimer->start(100);
m_Controls->m_TrackingControlLabel->setText("Status: tracking");
if (this->m_Controls->m_EnableLogging->isChecked()) StartLogging();
m_tracking=true;
}

void QmitkMITKIGTTrackingToolboxView::OnStopTracking()
{
if (!m_tracking) return;
m_TrackingTimer->stop();
m_TrackingDeviceSource->StopTracking();
m_TrackingDeviceSource->Disconnect();
this->m_Controls->m_configurationWidget->Reset();
m_Controls->m_TrackingControlLabel->setText("Status: stopped");
if (m_logging) StopLogging();
}

void QmitkMITKIGTTrackingToolboxView::MessageBox(std::string s)
  {
  QMessageBox msgBox;
  msgBox.setText(s.c_str());
  msgBox.exec();
  }

void QmitkMITKIGTTrackingToolboxView::UpdateTrackingTimer()
  {
  m_ToolVisualizationFilter->Update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  if (m_logging) this->m_loggingFilter->Update();
  }

void QmitkMITKIGTTrackingToolboxView::OnEnableLoggingClicked()
  {
  if (this->m_tracking && this->m_Controls->m_EnableLogging->isChecked() && !this->m_logging) StartLogging();
  else if (!this->m_Controls->m_EnableLogging->isChecked() && this->m_logging) StopLogging();
  }

void QmitkMITKIGTTrackingToolboxView::OnChooseFileClicked()
  {
  QString filename = QFileDialog::getSaveFileName(NULL,tr("Choose Logging File"), "/", "*.*");
  this->m_Controls->m_LoggingFileName->setText(filename);
  }

void QmitkMITKIGTTrackingToolboxView::StartLogging()
  {
  //initialize logging filter
  m_loggingFilter = mitk::NavigationDataRecorder::New();
  m_loggingFilter->SetRecordingMode(mitk::NavigationDataRecorder::NormalFile);
  if (m_Controls->m_xmlFormat->isChecked()) m_loggingFilter->SetOutputFormat(mitk::NavigationDataRecorder::xml);
  else if (m_Controls->m_csvFormat->isChecked()) m_loggingFilter->SetOutputFormat(mitk::NavigationDataRecorder::csv);
  m_loggingFilter->SetFileName(m_Controls->m_LoggingFileName->text().toStdString().c_str());
  
  //connect filter
  for(int i=0; i<m_ToolVisualizationFilter->GetNumberOfOutputs(); i++){m_loggingFilter->AddNavigationData(m_ToolVisualizationFilter->GetOutput(i));}
  
  m_loggingFilter->StartRecording();
  m_logging = true;
  }

void QmitkMITKIGTTrackingToolboxView::StopLogging()
  {
  m_loggingFilter->StopRecording();
  m_logging = false;
  }



