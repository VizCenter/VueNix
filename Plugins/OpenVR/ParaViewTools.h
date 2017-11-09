// ============================================================================
/**
 * @file   ParaViewTools.h
 *
 * @author nikhil shetty <nikhil.j.shetty@gmail.com>
 */
// ============================================================================
#ifndef __ParaViewTools_h
#define __ParaViewTools_h
// --------------------------------------------------------------------includes
#include <vtkSMViewProxy.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMSession.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMProxy.h>
// -------------------------------------------------------------------functions
vtkSMViewProxy* GetActiveViewProxy();
vtkSMRenderViewProxy* GetActiveRenderViewProxy();
vtkSMSession* GetActiveSession();
vtkSMSourceProxy* CreatePipelineProxy(vtkSMSession* session,
                                      const char* xmlgroup,
                                      const char* xmlname,
                                      vtkSMProxy* input=NULL);
void Show(vtkSMSourceProxy* proxy);
void Hide(vtkSMSourceProxy* proxy);

#endif
