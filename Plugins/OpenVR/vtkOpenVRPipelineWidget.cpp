/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOpenVRPipelineWidget.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkOpenVRPipelineWidget.h"
#include "vtkOpenVRPipelineRepresentation.h"

#include "vtkEventData.h"
#include "vtkNew.h"
#include "vtkCallbackCommand.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCamera.h"
#include "vtkObjectFactory.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"
#include "vtkInteractorStyle3D.h"
#include "vtkPropPicker.h"
#include "vtkAssemblyPath.h"
#include "vtkSMSourceProxy.h"

#include <map>

vtkStandardNewMacro(vtkOpenVRPipelineWidget);

class vtkOpenVRPipelineWidget::InternalElement
{
  public:
    vtkCommand *Command;
    std::string Name;
    std::string Text;
    bool visible;
    vtkSMSourceProxy *proxy;
    std::deque<InternalElement *> Pipe;

  InternalElement() {
  }
};

//----------------------------------------------------------------------
vtkOpenVRPipelineWidget::vtkOpenVRPipelineWidget()
{
  // Set the initial state
  this->WidgetState = vtkOpenVRPipelineWidget::Start;

  this->EventCommand = vtkCallbackCommand::New();
  this->EventCommand->SetClientData(this);
  this->EventCommand->SetCallback(vtkOpenVRPipelineWidget::EventCallback);

  {
    vtkNew<vtkEventDataButton3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    //ed->SetInput(vtkEventDataDeviceInput::ApplicationPipeline);
    ed->SetAction(vtkEventDataAction::Release);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Button3DEvent,
      ed, vtkWidgetEvent::Select,
      this, vtkOpenVRPipelineWidget::StartPipelineAction);
  }

  {
    vtkNew<vtkEventDataButton3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    ed->SetInput(vtkEventDataDeviceInput::Trigger);
    ed->SetAction(vtkEventDataAction::Release);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Button3DEvent,
      ed, vtkWidgetEvent::Select3D,
      this, vtkOpenVRPipelineWidget::SelectPipelineAction);
  }

  {
    vtkNew<vtkEventDataMove3D> ed;
    ed->SetDevice(vtkEventDataDevice::RightController);
    this->CallbackMapper->SetCallbackMethod(vtkCommand::Move3DEvent,
      ed, vtkWidgetEvent::Move3D,
      this, vtkOpenVRPipelineWidget::MoveAction);
  }
}

//----------------------------------------------------------------------
vtkOpenVRPipelineWidget::~vtkOpenVRPipelineWidget()
{
  this->EventCommand->Delete();
}

void vtkOpenVRPipelineWidget::PushFrontPipelineItem(
  const char *name,
  const char *text,
  vtkCommand *cmd)
{
  vtkOpenVRPipelineWidget::InternalElement *el =
    new vtkOpenVRPipelineWidget::InternalElement();
  el->Text = text;
  el->Command = cmd;
  el->Name = name;
  this->Pipe.push_front(el);

  static_cast<vtkOpenVRPipelineRepresentation *>(this->WidgetRep)->PushFrontPipelineItem(
        name, text, this->EventCommand);

  this->Modified();
}

void vtkOpenVRPipelineWidget::RenamePipelineItem(
  const char *name, const char *text)
{
  for (auto itr : this->Pipe)
  {
    if (itr->Name == name)
    {
      itr->Text = text;
    }
  }
  static_cast<vtkOpenVRPipelineRepresentation *>(this->WidgetRep)->
    RenamePipelineItem(name, text);
}

void vtkOpenVRPipelineWidget::RemovePipelineItem(
  const char *name)
{
  for (auto itr = this->Pipe.begin(); itr != this->Pipe.end(); ++itr)
  {
    if ((*itr)->Name == name)
    {
      delete *itr;
      this->Pipe.erase(itr);
      break;
    }
  }
  static_cast<vtkOpenVRPipelineRepresentation *>(this->WidgetRep)->
    RemovePipelineItem(name);
}

void vtkOpenVRPipelineWidget::RemoveAllPipelineItems()
{
  while (this->Pipe.size() > 0)
  {
    auto itr = this->Pipe.begin();
    delete *itr;
    this->Pipe.erase(itr);
  }
  static_cast<vtkOpenVRPipelineRepresentation *>(this->WidgetRep)->
    RemoveAllPipelineItems();
}


void vtkOpenVRPipelineWidget::EventCallback(
  vtkObject *,
  unsigned long,
  void *clientdata,
  void *calldata)
{
  vtkOpenVRPipelineWidget *self = static_cast<vtkOpenVRPipelineWidget *>(clientdata);
  std::string name = static_cast<const char *>(calldata);

  for (auto &pipeline : self->Pipe)
  {
    if (pipeline->Name == name)
    {
      pipeline->Command->Execute(self, vtkWidgetEvent::Select3D,
        static_cast<void *>(const_cast<char *>(pipeline->Name.c_str())));
    }
  }
}

//-------------------------------------------------------------------------
void vtkOpenVRPipelineWidget::ShowSubPipeline(vtkOpenVRPipelineWidget *w)
{
  w->SetInteractor(this->Interactor);
  w->Show(static_cast<vtkEventData *>(this->CallData));
}

void vtkOpenVRPipelineWidget::Show(vtkEventData *ed)
{
  this->On();
  if (this->WidgetState == vtkOpenVRPipelineWidget::Start)
  {
    if ( ! this->Parent )
    {
      this->GrabFocus(this->EventCallbackCommand);
    }
    this->CallData = ed;
    this->WidgetRep->StartComplexInteraction(
      this->Interactor, this, vtkWidgetEvent::Select, ed);

    this->WidgetState = vtkOpenVRPipelineWidget::Active;
  }
}

//-------------------------------------------------------------------------
void vtkOpenVRPipelineWidget::StartPipelineAction(vtkAbstractWidget *w)
{
  vtkOpenVRPipelineWidget *self = reinterpret_cast<vtkOpenVRPipelineWidget*>(w);

  if (self->WidgetState == vtkOpenVRPipelineWidget::Active)
  {
    if ( ! self->Parent )
    {
      self->ReleaseFocus();
    }

    self->Off();
    self->WidgetState = vtkOpenVRPipelineWidget::Start;

    self->WidgetRep->EndComplexInteraction(
      self->Interactor, self, vtkWidgetEvent::Select, self->CallData);
  }
}

//-------------------------------------------------------------------------
void vtkOpenVRPipelineWidget::SelectPipelineAction(vtkAbstractWidget *w)
{
  vtkOpenVRPipelineWidget *self = reinterpret_cast<vtkOpenVRPipelineWidget*>(w);

  if (self->WidgetState != vtkOpenVRPipelineWidget::Active)
  {
    return;
  }

  if ( ! self->Parent )
  {
    self->ReleaseFocus();
  }

  self->Off();
  self->WidgetState = vtkOpenVRPipelineWidget::Start;

  self->WidgetRep->ComplexInteraction(
    self->Interactor, self, vtkWidgetEvent::Select3D, self->CallData);
}

//-------------------------------------------------------------------------
void vtkOpenVRPipelineWidget::MoveAction(vtkAbstractWidget *w)
{
  vtkOpenVRPipelineWidget *self = reinterpret_cast<vtkOpenVRPipelineWidget*>(w);

  if (self->WidgetState != vtkOpenVRPipelineWidget::Active)
  {
    return;
  }

  self->WidgetRep->ComplexInteraction(
    self->Interactor, self, vtkWidgetEvent::Move3D, self->CallData);
}

//----------------------------------------------------------------------
void vtkOpenVRPipelineWidget::
SetRepresentation(vtkOpenVRPipelineRepresentation* rep)
{
  this->Superclass::SetWidgetRepresentation(
    reinterpret_cast<vtkWidgetRepresentation*>(rep));
}

//----------------------------------------------------------------------
void vtkOpenVRPipelineWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = vtkOpenVRPipelineRepresentation::New();
  }
}

//----------------------------------------------------------------------
void vtkOpenVRPipelineWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
