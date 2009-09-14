/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_UTIL_H_
#define _KJS_UTIL_H_

namespace kroll
{
	namespace KJSUtil
	{
		KROLL_JAVASCRIPT_API SharedValue ToKrollValue(JSValueRef, JSContextRef, JSObjectRef);
		KROLL_JAVASCRIPT_API JSValueRef ToJSValue(SharedValue, JSContextRef);
		KROLL_JAVASCRIPT_API JSValueRef KObjectToJSValue(SharedValue, JSContextRef);
		KROLL_JAVASCRIPT_API JSValueRef KMethodToJSValue(SharedValue, JSContextRef);
		KROLL_JAVASCRIPT_API JSValueRef KListToJSValue(SharedValue, JSContextRef);
		KROLL_JAVASCRIPT_API std::string ToChars(JSStringRef);
		KROLL_JAVASCRIPT_API bool IsArrayLike(JSObjectRef, JSContextRef);
		KROLL_JAVASCRIPT_API AutoPtr<KKJSObject> ToBoundObject(JSContextRef, JSObjectRef);
		KROLL_JAVASCRIPT_API AutoPtr<KKJSObject> ToBoundMethod(JSContextRef, JSObjectRef, JSObjectRef);
		KROLL_JAVASCRIPT_API AutoPtr<KKJSList> ToBoundList(JSContextRef, JSObjectRef);

		KROLL_JAVASCRIPT_API JSObjectRef CreateNewGlobalContext(Host*, bool addGlobalObject=true);
		KROLL_JAVASCRIPT_API void RegisterGlobalContext(JSObjectRef, JSGlobalContextRef);
		KROLL_JAVASCRIPT_API void UnregisterGlobalContext(JSObjectRef);
		KROLL_JAVASCRIPT_API JSGlobalContextRef GetGlobalContext(JSObjectRef);

		KROLL_JAVASCRIPT_API void ProtectGlobalContext(JSGlobalContextRef);
		KROLL_JAVASCRIPT_API void UnprotectGlobalContext(JSGlobalContextRef);

		KROLL_JAVASCRIPT_API SharedValue Evaluate(JSContextRef context, char* script);
		KROLL_JAVASCRIPT_API SharedValue EvaluateFile(JSContextRef context, std::string fullPath);
		KROLL_JAVASCRIPT_API void BindProperties(JSObjectRef, SharedKObject);
		KROLL_JAVASCRIPT_API SharedValue GetProperty(JSObjectRef, std::string name);
	};
}

#endif
