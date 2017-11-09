/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkImplicitPlaneRepresentation.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
* @class   vtkOpenVRPipelineRepresentation
* @brief   Widget representation for vtkOpenVRPanelWidget
* Implementation of the popup panel representation for the
* vtkOpenVRPanelWidget.
* This representation is rebuilt every time the selected/hovered prop changes.
* Its position is set according to the camera orientation and is placed at a
* distance defined in meters in the BuildRepresentation() method.
*
* WARNING: The panel might be occluded by other props.
*   TODO: Improve placement method.
**/

#ifndef vtkOpenVRPipelineRepresentation_h
#define vtkOpenVRPipelineRepresentation_h

#include "vtkRenderingOpenVRModule.h" // For export macro
#include "vtkWidgetRepresentation.h"
#include <deque> // for ivar

class vtkActor;
class vtkProperty;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkCellArray;
class vtkPoints;
class vtkTextActor3D;

class VTKRENDERINGOPENVR_EXPORT vtkOpenVRPipelineRepresentation : public vtkWidgetRepresentation
{
public:
  /**
  * Instantiate the class.
  */
  static vtkOpenVRPipelineRepresentation *New();

  //@{
  /**
  * Standard methods for the class.
  */
  vtkTypeMacro(vtkOpenVRPipelineRepresentation, vtkWidgetRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  //@}

  //@{
  /**
  * Methods to interface with the vtkOpenVRPanelWidget.
  */
  void BuildRepresentation() override;

  void StartComplexInteraction(
    vtkRenderWindowInteractor *iren,
    vtkAbstractWidget *widget,
    unsigned long event, void *calldata) override;
  void ComplexInteraction(
    vtkRenderWindowInteractor *iren,
    vtkAbstractWidget *widget,
    unsigned long event, void *calldata) override;
  void EndComplexInteraction(
    vtkRenderWindowInteractor *iren,
    vtkAbstractWidget *widget,
    unsigned long event, void *calldata) override;
  //@}

  //@{
  /**
  * Methods supporting the rendering process.
  */
  void ReleaseGraphicsResources(vtkWindow*) override;
  int HasTranslucentPolygonalGeometry() override;
  int RenderOverlay(vtkViewport*) override;
  //@}

  //@{
  /**
  * Methods to add/remove items to the pipeline, called by the pipeline widget
  */
  void PushFrontPipelineItem(const char *name, const char *text, vtkCommand *cmd);
  void RenamePipelineItem(const char *name, const char *text);
  void RemovePipelineItem(const char *name);
  void RemoveAllPipelineItems();
  //@}

  vtkGetMacro(CurrentOption, double);

protected:
  vtkOpenVRPipelineRepresentation();
  ~vtkOpenVRPipelineRepresentation() override;

  class InternalElement;
  std::deque<InternalElement *> Pipelines;

  double CurrentOption; // count from start of the list
  double PlacedPos[3];
  double PlacedDOP[3];
  double PlacedVUP[3];
  double PlacedVRight[3];
  double PlacedOrientation[3];

private:
  vtkOpenVRPipelineRepresentation(const vtkOpenVRPipelineRepresentation&) = delete;
  void operator=(const vtkOpenVRPipelineRepresentation&) = delete;
};

#endif
