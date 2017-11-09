// ============================================================================
/**
 * @file   ParaViewTools.cpp
 *
 * @author nikhil shetty <nikhil.j.shetty@gmail.com>
 */
// ============================================================================
#include "ParaViewTools.h"
#include "pqView.h"
#include "vtkSMViewProxy.h"
#include "pqActiveObjects.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkNew.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMParaViewPipelineController.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMParaViewPipelineControllerWithRendering.h"
// ----------------------------------------------------------------------------
//----------------------------------------------------------------------------
vtkSMViewProxy* GetActiveViewProxy()
{
  pqView* view = pqActiveObjects::instance().activeView();
  return view->getViewProxy();
}
//----------------------------------------------------------------------------
vtkSMRenderViewProxy* GetActiveRenderViewProxy()
{
  return vtkSMRenderViewProxy::SafeDownCast(GetActiveViewProxy());
}
//----------------------------------------------------------------------------
vtkSMSession* GetActiveSession()
{
  vtkSMSessionProxyManager* pxm = vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager();
  vtkSMSession* session = pxm->GetSession();
  return session;
}
//----------------------------------------------------------------------------
vtkSMSourceProxy* CreatePipelineProxy(vtkSMSession* session,
                                      const char* xmlgroup,
                                      const char* xmlname,
                                      vtkSMProxy* input)
{
  vtkSMSessionProxyManager* pxm = session->GetSessionProxyManager();
  vtkSmartPointer<vtkSMSourceProxy> proxy;
  proxy.TakeReference(vtkSMSourceProxy::SafeDownCast(pxm->NewProxy(xmlgroup, xmlname)));
  if (!proxy)
  {
    vtkGenericWarningMacro("Failed to create: " << xmlgroup << ", " << xmlname << ". Aborting !!!");
    abort();
  }

  vtkNew<vtkSMParaViewPipelineController> controller;
  controller->PreInitializeProxy(proxy.Get());
  if (input != NULL)
  {
    vtkSMPropertyHelper(proxy, "Input").Set(input);
  }
  controller->PostInitializeProxy(proxy.Get());
  proxy->UpdateVTKObjects();

  controller->RegisterPipelineProxy(proxy);
  return proxy.Get();
}
//----------------------------------------------------------------------------
void Show(vtkSMSourceProxy* proxy)
{
  vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
  vtkSMSession *session = GetActiveSession();
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
  controller->Show(proxy,0,GetActiveViewProxy());
}
//----------------------------------------------------------------------------
void Hide(vtkSMSourceProxy* proxy)
{
  vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
  vtkSMSession *session = GetActiveSession();
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
  controller->Hide(proxy,0,GetActiveViewProxy());
}
