/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkOpenVRPipelineWidget.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
* @class   vtkOpenVRPipelineWidget
* @brief   3D widget to display a pipeline in VR
*
*/

#ifndef vtkOpenVRPipelineWidget_h
#define vtkOpenVRPipelineWidget_h

#include "vtkRenderingOpenVRModule.h" // For export macro
#include "vtkAbstractWidget.h"
#include <deque> // for ivar

class vtkEventData;
class vtkOpenVRPipelineRepresentation;
class vtkPropMap;
class vtkProp;


class VTKRENDERINGOPENVR_EXPORT vtkOpenVRPipelineWidget : public vtkAbstractWidget
{
public:
  /**
  * Instantiate the object.
  */
  static vtkOpenVRPipelineWidget *New();

  //@{
  /**
  * Standard vtkObject methods
  */
  vtkTypeMacro(vtkOpenVRPipelineWidget, vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  /**
  * Specify an instance of vtkWidgetRepresentation used to represent this
  * widget in the scene. Note that the representation is a subclass of vtkProp
  * so it can be added to the renderer independent of the widget.
  */
  void SetRepresentation(vtkOpenVRPipelineRepresentation *rep);

  /**
  * Create the default widget representation if one is not set.
  */
  void CreateDefaultRepresentation() override;

  //@{
  /**
   * Get the widget state.
   */
  vtkGetMacro( WidgetState, int );
  //@}

  // Manage the state of the widget
  enum _WidgetState {Start=0,Active};

  /**
  * Create a tooltip associated to a prop.
  * Note that if the tooltip is already assigned to this prop,
  * its text will be replaced
  */
  void AddTooltip(vtkProp *prop, vtkStdString* str);
  void AddTooltip(vtkProp *prop, const char* str);

  //@{
  /**
  * Methods to add/remove items to the pipeline, called by the pipeline widget
  */
  void PushFrontPipelineItem(const char *name, const char *text, vtkCommand *cmd);
  void RenamePipelineItem(const char *name, const char *text);
  void RemovePipelineItem(const char *name);
  void RemoveAllPipelineItems();
  //@}

  void Show(vtkEventData *ed);
  void ShowSubPipeline(vtkOpenVRPipelineWidget *);

protected:
  vtkOpenVRPipelineWidget();
  ~vtkOpenVRPipelineWidget() override;

  int WidgetState;

  class InternalElement;
  std::deque<InternalElement *> Pipe;

  // These are the callbacks for this widget
  static void StartPipelineAction(vtkAbstractWidget*);
  static void SelectPipelineAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);

  vtkCallbackCommand* EventCommand;
  static void EventCallback(vtkObject* object,
                    unsigned long event,
                    void* clientdata,
                    void* calldata);

  /**
  * Update callback to check for the hovered prop
  */
  static void Update(vtkAbstractWidget*);

private:
  vtkOpenVRPipelineWidget(const vtkOpenVRPipelineWidget&) = delete;
  void operator=(const vtkOpenVRPipelineWidget&) = delete;
};
#endif
