/*=========================================================================

  Program:   ParaView
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVOpenVRHelper.h"

#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCompositeDataSet.h"
#include "vtkCullerCollection.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkDataSet.h"
#include "vtkDistanceRepresentation3D.h"
#include "vtkDistanceWidget.h"
#include "vtkGeometryRepresentation.h"
#include "vtkIdTypeArray.h"
#include "vtkImplicitPlaneRepresentation.h"
#include "vtkImplicitPlaneWidget2.h"
#include "vtkInformation.h"
#include "vtkLight.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenVRCamera.h"
#include "vtkOpenVRFollower.h"
#include "vtkOpenVRInteractorStyle.h"
#include "vtkOpenVRMenuRepresentation.h"
#include "vtkOpenVRMenuWidget.h"
#include "vtkOpenVROverlay.h"
#include "vtkOpenVRPanelRepresentation.h"
#include "vtkOpenVRPanelWidget.h"
#include "vtkOpenVRRenderWindow.h"
#include "vtkOpenVRRenderWindowInteractor.h"
#include "vtkOpenVRRenderer.h"
#include "vtkPointHandleRepresentation3D.h"
#include "vtkRenderViewBase.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkStringArray.h"
#include "vtkTextActor3D.h"
#include "vtkTextProperty.h"
#include "vtkVectorOperators.h"
#include "vtkWidgetEvent.h"

#include "vtkPVLODActor.h"
#include "vtkPVRenderView.h"

#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentedArrayListDomain.h"
#include "vtkSMViewProxy.h"

#include "vtkSMSessionProxyManager.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSmartPointer.h"
#include "vtkPVProxyDefinitionIterator.h"
#include "vtkPVXMLElement.h"
#include "vtkObjectFactory.h"
#include "vtkSMParaViewPipelineController.h"
#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkNew.h"
#include <sstream>
#include <string>
#include <set>
#include <stdlib.h>
#include <QCoreApplication>

vtkStandardNewMacro(vtkPVOpenVRHelper);
////-----------------------------------------------------------------------------
//void vtkPVOpenVRHelper::BuildSourceAndFilterMenu()
//{
//  std::set<std::string> ProxyDefinitionGroupToListen;
//  ProxyDefinitionGroupToListen.insert("sources");
//  ProxyDefinitionGroupToListen.insert("filters");

//  // Look inside the group name that are tracked
//  vtkSMSessionProxyManager* pxm =
//    vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();

//  if (ProxyDefinitionGroupToListen.size() == 0 || pxm == NULL)
//  {
//    return; // Nothing to look into...
//  }
//  vtkSMProxyDefinitionManager* pxdm = pxm->GetProxyDefinitionManager();

//  // Setup definition iterator
//  vtkSmartPointer<vtkPVProxyDefinitionIterator> iter;
//  iter.TakeReference(pxdm->NewIterator());
//  foreach (std::string groupName, ProxyDefinitionGroupToListen)
//  {
//    iter->AddTraversalGroupName(groupName.c_str());
//  }

//  // Loop over proxy that should be inserted inside the UI
//  std::set<std::pair<std::string, std::string> > definitionSet;
//  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
//  {
//    const char* group = iter->GetGroupName();
//    const char* name = iter->GetProxyName();
//    std::cout << "*******************************"<< std::endl;
//    std::cout << "Group : " <<group << std::endl;
//    std::cout << "Name : " << name << std::endl;
//    vtkPVXMLElement* hints = iter->GetProxyHints();
//    if (hints != NULL)
//    {
//      if (hints->FindNestedElementByName("ReaderFactory") != NULL)
//      {
//        // skip readers.
//        continue;
//      }
//      for (unsigned int cc = 0; cc < hints->GetNumberOfNestedElements(); cc++)
//      {
//        vtkPVXMLElement* showInMenu = hints->GetNestedElement(cc);
//        if (showInMenu == NULL || showInMenu->GetName() == NULL ||
//          strcmp(showInMenu->GetName(), "ShowInMenu") != 0)
//        {
//          continue;
//        }
//        std::cout << "******PLUGIN*************************"<< std::endl;
//        std::cout << "Group : " <<group << std::endl;
//        std::cout << "Name : " << name << std::endl;
//        definitionSet.insert(std::pair<std::string, std::string>(group, name));
//#if 0
//        this->Internal->addProxy(group, name, showInMenu->GetAttribute("icon"));
//        if (const char* categoryName = showInMenu->GetAttribute("category"))
//        {
//          pqInternal::CategoryInfo& category = this->Internal->Categories[categoryName];
//          // If no label just make it up
//          if (category.Label.isEmpty())
//          {
//            category.Label = categoryName;
//          }
//          int show_in_toolbar = 0;
//          if (showInMenu->GetScalarAttribute("show_in_toolbar", &show_in_toolbar))
//          {
//            category.ShowInToolbar = category.ShowInToolbar || (show_in_toolbar == 1);
//          }
//          if (!category.Proxies.contains(QPair<QString, QString>(group, name)))
//          {
//            category.Proxies.push_back(QPair<QString, QString>(group, name));
//          }
//        }
//#endif
//      }
//    }
//  }
//  // Update the menu with the current definition
//  // this->populateMenu();
//}
//----------------------------------------------------------------------------
vtkPVOpenVRHelper::vtkPVOpenVRHelper()
{
  this->EventCommand = vtkCallbackCommand::New();
  this->EventCommand->SetClientData(this);
  this->EventCommand->SetCallback(vtkPVOpenVRHelper::EventCallback);
  this->ScalarMenu->SetRepresentation(this->ScalarMenuRepresentation.Get());
  this->SourcesMenu->SetRepresentation(this->SourcesMenuRepresentation.Get());
  this->FiltersMenu->SetRepresentation(this->FiltersMenuRepresentation.Get());

  this->Renderer = nullptr;
  this->RenderWindow = nullptr;
  this->Style = nullptr;
  this->Interactor = nullptr;

  this->AddedProps = vtkPropCollection::New();
  this->NavRepresentation->GetTextActor()->GetTextProperty()->SetFontFamilyToCourier();
  this->NavRepresentation->GetTextActor()->GetTextProperty()->SetFrame(0);
  this->NavRepresentation->SetCoordinateSystemToLeftController();
}

//----------------------------------------------------------------------------
vtkPVOpenVRHelper::~vtkPVOpenVRHelper()
{
  this->EventCommand->Delete();
  this->AddedProps->Delete();
}

namespace
{
vtkPVDataRepresentation* FindRepresentation(vtkProp* prop, vtkView* view)
{
  int nr = view->GetNumberOfRepresentations();

  /**
   * The representation at a specified index.
   */
  vtkDataRepresentation* GetRepresentation(int index = 0);

  for (int i = 0; i < nr; ++i)
  {
    vtkGeometryRepresentation* gr =
      vtkGeometryRepresentation::SafeDownCast(view->GetRepresentation(i));
    if (gr && gr->GetActor() == prop)
    {
      return gr;
    }
  }
  return NULL;
}
}
//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::GetScalars()
{
  this->ScalarMap.clear();

  vtkSMPropertyHelper helper(this->SMView, "Representations");
  for (unsigned int i = 0; i < helper.GetNumberOfElements(); i++)
  {
    vtkSMPVRepresentationProxy* repr =
      vtkSMPVRepresentationProxy::SafeDownCast(helper.GetAsProxy(i));
    vtkSMProperty* prop = repr ? repr->GetProperty("ColorArrayName") : nullptr;
    if (prop)
    {
      vtkSMRepresentedArrayListDomain* scalars = vtkSMRepresentedArrayListDomain::SafeDownCast(
        prop->FindDomain("vtkSMRepresentedArrayListDomain"));
      int numsc = scalars->GetNumberOfStrings();
      for (int i = 0; i < numsc; ++i)
      {
        std::string name = scalars->GetString(i);
        int assoc = scalars->GetFieldAssociation(i);
        if (assoc != vtkDataObject::FIELD && this->ScalarMap.find(name) == this->ScalarMap.end())
        {
          this->ScalarMap.insert(std::pair<std::string, int>(name, assoc));
        }
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::GetSources()
{
  this->SourcesMap.clear();
  this->SourcesMap.insert(std::pair<std::string, int>("VectorText", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("ArrowSource", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("Axes", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("CubeSource", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("ConeSource", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("CylinderSource", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("LineSource", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("Ruler", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("PlaneSource", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("PointSource", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("SphereSource", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("DiskSource", 1));
  this->SourcesMap.insert(std::pair<std::string, int>("RTAnalyticSource", 1));

}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::GetFilters()
{
  this->FiltersMap.clear();
  this->FiltersMap.insert(std::pair<std::string, int>("GenericContour", 1));
  this->FiltersMap.insert(std::pair<std::string, int>("GenericClip", 2));
  this->FiltersMap.insert(std::pair<std::string, int>("GenericCut", 3));
  this->FiltersMap.insert(std::pair<std::string, int>("Threshold", 4));
  this->FiltersMap.insert(std::pair<std::string, int>("ExtractGrid", 5));
  this->FiltersMap.insert(std::pair<std::string, int>("GenericGeometryFilter", 6));
  this->FiltersMap.insert(std::pair<std::string, int>("GenericStreamTracer", 7));
  this->FiltersMap.insert(std::pair<std::string, int>("GenericOutlineFilter", 8));
  this->FiltersMap.insert(std::pair<std::string, int>("GenericTessellator", 9));
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::BuildScalarMenu()
{
  this->ScalarMenu->PushFrontMenuItem("exit", "Exit", this->EventCommand);

  int count = 0;
  for (auto i : this->ScalarMap)
  {
    std::string tmp = i.first;
    if (i.second == vtkDataObject::POINT)
    {
      tmp += " (point)";
    }
    if (i.second == vtkDataObject::CELL)
    {
      tmp += " (cell)";
    }

    std::ostringstream toString;
    toString << count;

    this->ScalarMenu->PushFrontMenuItem(toString.str().c_str(), tmp.c_str(), this->EventCommand);
    count++;
  }
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::BuildSourcesMenu()
{
    this->SourcesMenu->PushFrontMenuItem("exit", "Exit", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("RTAnalyticSource", "Wavelet", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("SphereSource", "Sphere", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("Ruler", "Ruler", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("PointSource", "Point", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("PlaneSource", "Plane", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("LineSource", "Line", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("DiskSource", "Disk", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("CylinderSource", "Cylinder", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("ConeSource", "Cone", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("CubeSource", "Box", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("Axes", "Axes", this->EventCommand);
    this->SourcesMenu->PushFrontMenuItem("ArrowSource", "Arrow", this->EventCommand);
    this->FiltersMenu->PushFrontMenuItem("VectorText", "3D Text", this->EventCommand);
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::BuildFiltersMenu()
{
  this->FiltersMenu->PushFrontMenuItem("exit", "Exit", this->EventCommand);
  this->FiltersMenu->PushFrontMenuItem("ProbePoint", "Probe Location", this->EventCommand);
  this->FiltersMenu->PushFrontMenuItem("GenericStreamTracer", "Stream Tracer", this->EventCommand);
  this->FiltersMenu->PushFrontMenuItem("Glyph", "Glyph", this->EventCommand);
  this->FiltersMenu->PushFrontMenuItem("ExtractGrid", "Extract Subset", this->EventCommand);
  this->FiltersMenu->PushFrontMenuItem("Threshold", "Threshold", this->EventCommand);
  this->FiltersMenu->PushFrontMenuItem("GenericCut", "Cut", this->EventCommand);
  this->FiltersMenu->PushFrontMenuItem("GenericClip", "Clip", this->EventCommand);
  this->FiltersMenu->PushFrontMenuItem("GenericContour", "Contour", this->EventCommand);
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::ToggleNavigationPanel()
{
  if (!this->NavWidget->GetEnabled())
  {
    // add an observer on the left controller to update the bearing and position
    this->NavigationTag =
      this->Interactor->AddObserver(vtkCommand::Move3DEvent, this->EventCommand, 1.0);

    this->NavWidget->SetInteractor(this->Interactor);
    this->NavWidget->SetRepresentation(this->NavRepresentation.Get());
    this->NavRepresentation->SetText("\n Position not updated yet \n");
    double scale = this->RenderWindow->GetPhysicalScale();

    double bnds[6] = { -0.3, 0.3, 0.01, 0.01, -0.01, -0.01 };
    double normal[3] = { 0, 2, 1 };
    double vup[3] = { 0, 1, -2 };
    this->NavRepresentation->PlaceWidgetExtended(bnds, normal, vup, scale);

    this->NavWidget->SetEnabled(1);
  }
  else
  {
    this->NavWidget->SetEnabled(0);
    this->Renderer->RemoveObserver(this->NavigationTag);
  }
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::AddACropPlane()
{
  vtkNew<vtkImplicitPlaneRepresentation> rep;
  rep->SetHandleSize(15.0);
  rep->SetDrawOutline(0);
  rep->GetPlaneProperty()->SetOpacity(0.01);
  rep->ConstrainToWidgetBoundsOff();
  // rep->SetPlaceFactor(1.25);
  double* fp = this->Renderer->GetActiveCamera()->GetFocalPoint();
  double scale = this->RenderWindow->GetPhysicalScale();
  double bnds[6] = { fp[0] - scale * 0.5, fp[0] + scale * 0.5, fp[1] - scale * 0.5,
    fp[1] + scale * 0.5, fp[2] - scale * 0.5, fp[2] + scale * 0.5 };
  rep->PlaceWidget(bnds);
  rep->SetOrigin(fp);
  rep->SetNormal(this->Renderer->GetActiveCamera()->GetDirectionOfProjection());

  vtkNew<vtkImplicitPlaneWidget2> ps;
  this->CropPlanes.insert(ps.Get());
  ps->Register(this);

  ps->SetRepresentation(rep.Get());
  ps->SetInteractor(this->Interactor);
  ps->SetEnabled(1);

  vtkCollectionSimpleIterator pit;
  vtkProp* prop;
  vtkActor* aPart;
  vtkAssemblyPath* path;
  for (this->AddedProps->InitTraversal(pit); (prop = this->AddedProps->GetNextProp(pit));)
  {
    for (prop->InitPathTraversal(); (path = prop->GetNextPath());)
    {
      aPart = static_cast<vtkActor*>(path->GetLastNode()->GetViewProp());
      if (aPart->GetMapper())
      {
        aPart->GetMapper()->AddClippingPlane(rep->GetUnderlyingPlane());
        continue;
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::RemoveAllCropPlanes()
{
  for (vtkImplicitPlaneWidget2* iter : this->CropPlanes)
  {
    iter->SetEnabled(0);

    vtkImplicitPlaneRepresentation* rep =
      static_cast<vtkImplicitPlaneRepresentation*>(iter->GetRepresentation());

    vtkCollectionSimpleIterator pit;
    vtkProp* prop;
    vtkActor* aPart;
    vtkAssemblyPath* path;
    for (this->AddedProps->InitTraversal(pit); (prop = this->AddedProps->GetNextProp(pit));)
    {
      for (prop->InitPathTraversal(); (path = prop->GetNextPath());)
      {
        aPart = static_cast<vtkActor*>(path->GetLastNode()->GetViewProp());
        if (aPart->GetMapper())
        {
          aPart->GetMapper()->RemoveClippingPlane(rep->GetUnderlyingPlane());
          continue;
        }
      }
    }

    iter->UnRegister(this);
  }
  this->CropPlanes.clear();
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::SelectScalar(std::string name)
{
  std::istringstream is(name);
  int target;
  is >> target;

  int count = 0;
  std::string tname;
  int tassoc = -1;
  for (auto i : this->ScalarMap)
  {
    if (count == target)
    {
      tname = i.first;
      tassoc = i.second;
      break;
    }
    count++;
  }

  vtkSMPropertyHelper helper(this->SMView, "Representations");
  for (unsigned int i = 0; i < helper.GetNumberOfElements(); i++)
  {
    vtkSMPVRepresentationProxy* repr =
      vtkSMPVRepresentationProxy::SafeDownCast(helper.GetAsProxy(i));
    vtkSMProperty* prop = repr ? repr->GetProperty("ColorArrayName") : nullptr;
    if (prop)
    {
      vtkSMRepresentedArrayListDomain* scalars = vtkSMRepresentedArrayListDomain::SafeDownCast(
        prop->FindDomain("vtkSMRepresentedArrayListDomain"));
      int numsc = scalars->GetNumberOfStrings();
      bool found = false;
      for (int i = 0; i < numsc && !found; ++i)
      {
        std::string sname = scalars->GetString(i);
        int assoc = scalars->GetFieldAssociation(i);
        if (assoc == tassoc && sname == tname)
        {
          // vtkSMPVRepresentationProxy - has method SetColorArrayName
          repr->SetScalarColoring(tname.c_str(), tassoc);
          found = true;
        }
      }
      if (!found)
      {
        // solid color
        repr->SetScalarColoring(nullptr, tassoc);
      }
    }
  }
  this->SMView->StillRender();
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::SelectSources(std::string name)
{
    vtkSMSessionProxyManager* pxm = vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();
    vtkSMSession* session = pxm->GetSession();

    vtkNew<vtkSMParaViewPipelineController> controller;
    if (pxm == NULL)
    {
      std::cout << "Proxy Manager not found"<<std::endl;
      return; // Nothing to look into...
    }
    if (!controller->InitializeSession(session))
    {
      cerr << "Failed to initialize ParaView session." << endl;
      return;
    }

    if (controller->FindTimeKeeper(session) == NULL)
    {
      cerr << "Failed at line " << __LINE__ << endl;
      return;
    }

    if (controller->FindAnimationScene(session) == NULL)
    {
      cerr << "Failed at line " << __LINE__ << endl;
      return;
    }

    if (controller->GetTimeAnimationTrack(controller->GetAnimationScene(session)) == NULL)
    {
      cerr << "Failed at line " << __LINE__ << endl;
      return;
    }

      vtkSmartPointer<vtkSMSourceProxy> source;
        source.TakeReference(
          vtkSMSourceProxy::SafeDownCast(pxm->NewProxy("sources", name.c_str())));
        //vtkSMPropertyHelper(sphereSource, "Radius").Set(10);
      controller->PreInitializeProxy(source);

 //      pxm->RegisterProxy("sources",name.c_str(),source);
//        vtkSmartPointer<vtkSMProxy> view;
//        vtkSmartPointer<vtkSMProxy> repr;

//        view.TakeReference(pxm->GeRenderView->NewProxy("views", "RenderView"));

        source->UpdateVTKObjects();
        //source->UpdatePipelineInformation();

        controller->PostInitializeProxy(source);
        controller->RegisterPipelineProxy(source);
        // Create view
            vtkSmartPointer<vtkSMProxy> view;
            view.TakeReference(this->SMView);
//            controller->PreInitializeProxy(view);
//            controller->PostInitializeProxy(view);
//            controller->RegisterViewProxy(view);

#if 0
            vtkSMProxy* repr = this->SMView->FindRepresentation(source,1 );
            if(repr== NULL){
                        cerr << "Failed at line " << __LINE__ << endl;
                    return;
                    }
#else
        vtkSmartPointer<vtkSMProxy> repr;
        repr.TakeReference(
          vtkSMViewProxy::SafeDownCast(view)->CreateDefaultRepresentation(source, 0));
        if(repr== NULL){
            cerr << "Failed at line " << __LINE__ << endl;
        return;
        }
        controller->PreInitializeProxy(repr);
        vtkSMPropertyHelper(repr, "Input").Set(source);
        controller->PostInitializeProxy(repr);
        controller->RegisterRepresentationProxy(repr);

        vtkSMPropertyHelper(view, "Representations").Add(repr);
#endif
        //this->SMView->FindRepresentation(source,1)
        vtkSMPropertyHelper(repr, "Visibility").Set(1);
        //view->UpdateVTKObjects();
       // controller->Show(source,1,this->SMView);

         //   source->UpdatePipeline();
        //this->SendToOpenVR(this->SMView);

        //this->UpdateProps();
        this->SMView->StillRender();
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::SelectFilters(std::string name)
{
  std::istringstream is(name);
  int target;
  is >> target;

  int count = 0;
  std::string tname;
  int tassoc = -1;
  for (auto i : this->ScalarMap)
  {
    if (count == target)
    {
      tname = i.first;
      tassoc = i.second;
      break;
    }
    count++;
  }

  vtkSMPropertyHelper helper(this->SMView, "Representations");
  for (unsigned int i = 0; i < helper.GetNumberOfElements(); i++)
  {
    vtkSMPVRepresentationProxy* repr =
      vtkSMPVRepresentationProxy::SafeDownCast(helper.GetAsProxy(i));
    vtkSMProperty* prop = repr ? repr->GetProperty("ColorArrayName") : nullptr;
    if (prop)
    {
      vtkSMRepresentedArrayListDomain* scalars = vtkSMRepresentedArrayListDomain::SafeDownCast(
        prop->FindDomain("vtkSMRepresentedArrayListDomain"));
      int numsc = scalars->GetNumberOfStrings();
      bool found = false;
      for (int i = 0; i < numsc && !found; ++i)
      {
        std::string sname = scalars->GetString(i);
        int assoc = scalars->GetFieldAssociation(i);
        if (assoc == tassoc && sname == tname)
        {
          // vtkSMPVRepresentationProxy - has method SetColorArrayName
          repr->SetScalarColoring(tname.c_str(), tassoc);
          found = true;
        }
      }
      if (!found)
      {
        // solid color
        repr->SetScalarColoring(nullptr, tassoc);
      }
    }
  }
  this->SMView->StillRender();
}


//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::HandleMenuEvent(vtkOpenVRMenuWidget* menu,
                                        vtkObject*,
                                        unsigned long eventID,
                                        void*,
                                        void* calldata)
{
  // handle menu events
  if (menu == this->Style->GetMenu() && eventID == vtkWidgetEvent::Select3D)
  {
    std::string name = static_cast<const char*>(calldata);

    if (name == "takemeasurement")
    {
      this->DistanceWidget->SetWidgetStateToStart();
      this->DistanceWidget->SetEnabled(0);
      this->DistanceWidget->SetEnabled(1);
    }
    if (name == "togglenavigationpanel")
    {
      this->ToggleNavigationPanel();
    }
    if (name == "selectscalar")
    {
      menu->ShowSubMenu(this->ScalarMenu.Get());
    }
    if (name == "selectsources")
    {
      menu->ShowSubMenu(this->SourcesMenu.Get());
    }
    if (name == "selectfilters")
    {
      menu->ShowSubMenu(this->FiltersMenu.Get());
    }
    if (name == "addacropplane")
    {
      this->AddACropPlane();
    }
    if (name == "removeallcropplanes")
    {
      this->RemoveAllCropPlanes();
    }

    return;
  }

  if (menu == this->ScalarMenu.Get() && eventID == vtkWidgetEvent::Select3D)
  {
    std::string name = static_cast<const char*>(calldata);

    if (name != "exit")
    {
      this->SelectScalar(name);
    }
    return;
  }

  if (menu == this->ScalarMenu.Get() && eventID == vtkWidgetEvent::Select)
  {
    this->Style->GetMenu()->On();
  }
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::HandleInteractorEvent(vtkOpenVRRenderWindowInteractor*,
                                              vtkObject*,
                                              unsigned long eventID,
                                              void*, void* calldata)
{
  vtkEventData* edata = static_cast<vtkEventData*>(calldata);
  vtkEventDataDevice3D* edd = edata->GetAsEventDataDevice3D();

  if (edd && edd->GetDevice() == vtkEventDataDevice::LeftController &&
    this->NavWidget->GetEnabled() && eventID == vtkCommand::Move3DEvent /* &&
      cam->GetLeftEye() */)
  {
    double pos[4];
    edd->GetWorldPosition(pos);
    pos[3] = 1.0;

    // use scale to control resolution, we want to show about
    // 2mm of resolution
    double scale = this->RenderWindow->GetPhysicalScale();
    double sfactor = pow(10.0, floor(log10(scale * 0.002)));
    pos[0] = floor(pos[0] / sfactor) * sfactor;
    pos[1] = floor(pos[1] / sfactor) * sfactor;
    pos[2] = floor(pos[2] / sfactor) * sfactor;
    std::ostringstream toString;
    toString << std::resetiosflags(std::ios::adjustfield);
    toString << std::setiosflags(std::ios::left);
    toString << setprecision(7);
    toString << "\n Position:\n " << setw(8) << pos[0] << ", " << setw(8) << pos[1] << ", "
             << setw(8) << pos[2] << " \n";

    // compute the bearing, the bearing is the angle in
    // the XY plane
    edd->GetWorldDirection(pos);
    vtkVector3d vup(0, 0, 1);
    vtkVector3d vdir(0, 1, 0);
    vtkVector3d bear(pos);

    // remove any up component
    double upc = vup.Dot(bear);
    if (fabs(upc) < 1.0)
    {
      bear = bear - vup * upc;
      bear.Normalize();

      double theta = acos(bear.Dot(vdir));
      if (vup.Cross(bear).Dot(vdir) < 0.0)
      {
        theta = -theta;
      }
      theta = vtkMath::DegreesFromRadians(theta);
      theta = 0.1 * floor(theta * 10);
      toString << std::fixed;
      toString << setprecision(1);
      toString << " Bearing: " << setw(5) << theta << " East of North \n";
    }
    else
    {
      toString << " Bearing: undefined \n";
    }
    this->NavRepresentation->SetText(toString.str().c_str());
  }
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::EventCallback(vtkObject* caller,
                                      unsigned long eventID,
                                      void* clientdata,
                                      void* calldata)
{
  vtkPVOpenVRHelper* self = static_cast<vtkPVOpenVRHelper*>(clientdata);

  vtkOpenVRRenderWindowInteractor* iren = vtkOpenVRRenderWindowInteractor::SafeDownCast(caller);
  if (iren)
  {
    self->HandleInteractorEvent(iren, caller, eventID, clientdata, calldata);
    return;
  }

  vtkOpenVRMenuWidget* menu = vtkOpenVRMenuWidget::SafeDownCast(caller);
  if (menu)
  {
    self->HandleMenuEvent(menu, caller, eventID, clientdata, calldata);
    return;
  }

  // handle different events
  switch (eventID)
  {
    case vtkCommand::EndPickEvent:
    {
      vtkSelection* sel = vtkSelection::SafeDownCast(reinterpret_cast<vtkObjectBase*>(calldata));

      if (!sel)
      {
        self->DistanceWidget->SetWidgetStateToStart();
        self->DistanceWidget->SetEnabled(0);
        return;
      }

      vtkOpenVRInteractorStyle* is =
        vtkOpenVRInteractorStyle::SafeDownCast(reinterpret_cast<vtkObjectBase*>(caller));

      vtkSelectionNode* node = sel->GetNode(0);
      if (!node || !node->GetProperties()->Has(vtkSelectionNode::PROP_ID()))
      {
        return;
      }

      vtkProp3D* prop =
        vtkProp3D::SafeDownCast(node->GetProperties()->Get(vtkSelectionNode::PROP()));
      vtkPVDataRepresentation* repr = FindRepresentation(prop, self->View);
      if (!repr)
      {
        return;
      }

      vtkDataObject* dobj = repr->GetInput();
      node->GetProperties()->Set(vtkSelectionNode::SOURCE(), repr);

      vtkCompositeDataSet* cds = vtkCompositeDataSet::SafeDownCast(dobj);
      vtkDataSet* ds = NULL;
      // handle composite datasets
      if (cds)
      {
        vtkIdType cid = node->GetProperties()->Get(vtkSelectionNode::COMPOSITE_INDEX());
        vtkNew<vtkDataObjectTreeIterator> iter;
        iter->SetDataSet(cds);
        iter->SkipEmptyNodesOn();
        iter->SetVisitOnlyLeaves(1);
        iter->InitTraversal();
        while (iter->GetCurrentFlatIndex() != cid && !iter->IsDoneWithTraversal())
        {
          iter->GoToNextItem();
        }
        if (iter->GetCurrentFlatIndex() == cid)
        {
          ds = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
        }
      }
      else
      {
        ds = vtkDataSet::SafeDownCast(dobj);
      }
      if (!ds)
      {
        return;
      }

      // get the picked cell
      vtkIdTypeArray* ids = vtkArrayDownCast<vtkIdTypeArray>(node->GetSelectionList());
      if (ids == 0)
      {
        return;
      }
      vtkIdType aid = ids->GetComponent(0, 0);
      vtkCell* cell = ds->GetCell(aid);

      // update the billboard with information about this data
      std::ostringstream toString;
      double p1[6];
      cell->GetBounds(p1);
      double pos[3] = { 0.5 * (p1[1] + p1[0]), 0.5 * (p1[3] + p1[2]), 0.5 * (p1[5] + p1[4]) };
      toString << "\n Cell Center (DC): " << pos[0] << ", " << pos[1] << ", " << pos[2] << " \n";
      vtkMatrix4x4* pmat = prop->GetMatrix();
      double wpos[4];
      wpos[0] = pos[0];
      wpos[1] = pos[1];
      wpos[2] = pos[2];
      wpos[3] = 1.0;
      pmat->MultiplyPoint(wpos, wpos);
      toString << " Cell Center (WC): " << wpos[0] << ", " << wpos[1] << ", " << wpos[2] << " \n";

      vtkCellData* celld = ds->GetCellData();
      vtkIdType numcd = celld->GetNumberOfArrays();
      for (vtkIdType i = 0; i < numcd; ++i)
      {
        vtkAbstractArray* aa = celld->GetAbstractArray(i);
        if (aa && aa->GetName())
        {
          toString << " " << aa->GetName() << ": ";
          vtkStringArray* sa = vtkStringArray::SafeDownCast(aa);
          if (sa)
          {
            toString << sa->GetValue(aid) << " \n";
          }
          vtkDataArray* da = vtkDataArray::SafeDownCast(aa);
          if (da)
          {
            int nc = da->GetNumberOfComponents();
            for (int ci = 0; ci < nc; ++ci)
            {
              toString << da->GetComponent(aid, ci) << " ";
            }
            toString << "\n";
          }
        }
      }
      toString << "\n";

      is->ShowBillboard(toString.str());
      is->ShowPickCell(cell, vtkProp3D::SafeDownCast(prop));
    }
    break;
  }
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::ViewRemoved(vtkSMViewProxy* smview)
{
  // if this is not our view then we don't care
  if (this->SMView != smview)
  {
    return;
  }

  this->Quit();
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::SendToOpenVR(vtkSMViewProxy* smview)
{
  this->SMView = smview;
  this->View = vtkPVRenderView::SafeDownCast(smview->GetClientSideView());

  if (!this->View)
  {
    vtkErrorMacro("Send to VR without a valid view");
    return;
  }

  // are we already in VR ?
  if (this->Interactor)
  {
    // just update the actors and return
    this->UpdateProps();
    return;
  }

  vtkRenderer* pvRenderer = this->View->GetRenderView()->GetRenderer();
  vtkRenderWindow* pvRenderWindow = vtkRenderWindow::SafeDownCast(pvRenderer->GetVTKWindow());

  this->RenderWindow = vtkOpenVRRenderWindow::New();
  this->Renderer = vtkOpenVRRenderer::New();
  this->RenderWindow->AddRenderer(this->Renderer);
  this->Interactor = vtkOpenVRRenderWindowInteractor::New();
  this->RenderWindow->SetInteractor(this->Interactor);
  vtkOpenVRCamera* cam = vtkOpenVRCamera::New();
  this->Renderer->SetActiveCamera(cam);
  cam->Delete();

  // iren->SetDesiredUpdateRate(220.0);
  // iren->SetStillUpdateRate(220.0);
  // renWin->SetDesiredUpdateRate(220.0);

  // add a pick observer
  this->Style = vtkOpenVRInteractorStyle::SafeDownCast(this->Interactor->GetInteractorStyle());
  this->Style->AddObserver(vtkCommand::EndPickEvent, this->EventCommand, 1.0);
  this->Style->GetMenu()->PushFrontMenuItem(
    "takemeasurement", "Take a measurement", this->EventCommand);
  this->Style->GetMenu()->PushFrontMenuItem(
    "togglenavigationpanel", "Toggle the Navigation Panel", this->EventCommand);
  this->Style->GetMenu()->PushFrontMenuItem(
    "removeallcropplanes", "Remove all Crop Planes", this->EventCommand);
  this->Style->GetMenu()->PushFrontMenuItem(
    "addacropplane", "Add a Crop Plane", this->EventCommand);
  this->Style->GetMenu()->PushFrontMenuItem(
    "selectscalar", "Select Scalar to View", this->EventCommand);
  this->Style->GetMenu()->PushFrontMenuItem(
    "selectfilters", "Filters Menu", this->EventCommand);
  this->Style->GetMenu()->PushFrontMenuItem(
    "selectsources", "Sources Menu", this->EventCommand);
  // get the scalars
  this->GetScalars();
  this->BuildScalarMenu();

  // build sources menu and filter menu
  this->GetSources();
  this->BuildSourcesMenu();

  this->GetFilters();
  this->BuildFiltersMenu();

  this->Renderer->RemoveCuller(this->Renderer->GetCullers()->GetLastItem());
  this->Renderer->SetBackground(pvRenderer->GetBackground());

  this->DistanceWidget = vtkDistanceWidget::New();
  this->DistanceWidget->SetInteractor(this->Interactor);
  vtkNew<vtkDistanceRepresentation3D> drep;
  this->DistanceWidget->SetRepresentation(drep.Get());
  vtkNew<vtkOpenVRFollower> fol;
  drep->SetLabelActor(fol.Get());
  vtkNew<vtkPointHandleRepresentation3D> hr;
  drep->SetHandleRepresentation(hr.Get());
  hr->SetHandleSize(40);

  drep->SetLabelFormat("Dist: %g\ndeltaX: %g\ndeltaY: %g\ndeltaZ: %g");
  // create 4 lights for even lighting
  {
    vtkLight* light = vtkLight::New();
    light->SetPosition(0.0, 1.0, 0.0);
    light->SetIntensity(1.0);
    light->SetLightTypeToSceneLight();
    this->Renderer->AddLight(light);
    light->Delete();
  }
  {
    vtkLight* light = vtkLight::New();
    light->SetPosition(0.8, -0.2, 0.0);
    light->SetIntensity(0.8);
    light->SetLightTypeToSceneLight();
    this->Renderer->AddLight(light);
    light->Delete();
  }
  {
    vtkLight* light = vtkLight::New();
    light->SetPosition(-0.3, -0.2, 0.7);
    light->SetIntensity(0.6);
    light->SetLightTypeToSceneLight();
    this->Renderer->AddLight(light);
    light->Delete();
  }
  {
    vtkLight* light = vtkLight::New();
    light->SetPosition(-0.3, -0.2, -0.7);
    light->SetIntensity(0.4);
    light->SetLightTypeToSceneLight();
    this->Renderer->AddLight(light);
    light->Delete();
  }

// send the first renderer to openVR

#if 1
  vtkNew<vtkCamera> zcam;
  double fp[3];
  pvRenderer->GetActiveCamera()->GetFocalPoint(fp);
  zcam->SetFocalPoint(fp);
  zcam->SetViewAngle(pvRenderer->GetActiveCamera()->GetViewAngle());
  zcam->SetViewUp(0.0, 0.0, 1.0);
  double distance = pvRenderer->GetActiveCamera()->GetDistance();
  zcam->SetPosition(fp[0] + distance, fp[1], fp[2]);
  this->RenderWindow->InitializeViewFromCamera(zcam.Get());
#else
  renWin->InitializeViewFromCamera(pvRenderer->GetActiveCamera());
#endif

  if (this->NextXML.length())
  {
    std::istringstream iss(this->NextXML);
    this->RenderWindow->GetDashboardOverlay()->ReadCameraPoses(iss);
  }
  else
  {
    this->RenderWindow->GetDashboardOverlay()->ReadCameraPoses();
  }
  vtkActorCollection* acol = pvRenderer->GetActors();
  vtkCollectionSimpleIterator pit;
  vtkActor* actor;
  this->AddedProps->RemoveAllItems();
  for (acol->InitTraversal(pit); (actor = acol->GetNextActor(pit));)
  {
    this->AddedProps->AddItem(actor);
    this->Renderer->AddActor(actor);
  }
  vtkVolumeCollection* avol = pvRenderer->GetVolumes();
  vtkVolume* volume;
  for (avol->InitTraversal(pit); (volume = avol->GetNextVolume(pit));)
  {
    this->AddedProps->AddItem(volume);
    this->Renderer->AddVolume(volume);
  }
  // use MSAA if no volumes
  this->RenderWindow->SetMultiSamples(avol->GetNumberOfItems() ? 0 : 8);

  // test out sharing the context
  this->RenderWindow->SetHelperWindow(static_cast<vtkOpenGLRenderWindow*>(pvRenderWindow));

  this->RenderWindow->Initialize();

  if (this->RenderWindow->GetHMD())
  {
    this->RenderWindow->Render();
    this->Renderer->ResetCamera();
    this->Renderer->ResetCameraClippingRange();
    while (this->Interactor && !this->Interactor->GetDone())
    {
      this->Interactor->DoOneEvent(this->RenderWindow, this->Renderer);
      QCoreApplication::processEvents();
    }
  }

  std::ostringstream oss;
  this->RenderWindow->GetDashboardOverlay()->WriteCameraPoses(oss);
  this->NextXML = oss.str();

  this->AddedProps->RemoveAllItems();
  this->DistanceWidget->Delete(); // must delete before the interactor
  this->Renderer->Delete();
  this->Renderer = nullptr;
  this->Interactor->Delete();
  this->Interactor = nullptr;
  this->RenderWindow->SetHelperWindow(nullptr);
  this->RenderWindow->Delete();
  this->RenderWindow = nullptr;
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::UpdateProps()
{
  if (!this->Renderer)
  {
    return;
  }

  vtkRenderer* pvRenderer = this->View->GetRenderView()->GetRenderer();
  if (pvRenderer->GetViewProps()->GetMTime() > this->PropUpdateTime)
  {
    // remove prior props
    vtkCollectionSimpleIterator pit;
    vtkProp* prop;
    for (this->AddedProps->InitTraversal(pit); (prop = this->AddedProps->GetNextProp(pit));)
    {
      this->Renderer->RemoveViewProp(prop);
    }

    vtkActorCollection* acol = pvRenderer->GetActors();
    vtkActor* actor;
    this->AddedProps->RemoveAllItems();
    for (acol->InitTraversal(pit); (actor = acol->GetNextActor(pit));)
    {
      this->AddedProps->AddItem(actor);
      this->Renderer->AddActor(actor);
    }
    vtkVolumeCollection* avol = pvRenderer->GetVolumes();
    vtkVolume* volume;
    for (avol->InitTraversal(pit); (volume = avol->GetNextVolume(pit));)
    {
      this->AddedProps->AddItem(volume);
      this->Renderer->AddVolume(volume);
    }

    this->PropUpdateTime.Modified();
  }

  this->Interactor->DoOneEvent(this->RenderWindow, this->Renderer);
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::Quit()
{
  if (this->Interactor)
  {
    this->Interactor->TerminateApp();
  }
}

//----------------------------------------------------------------------------
void vtkPVOpenVRHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
