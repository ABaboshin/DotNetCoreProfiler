using System.Collections.Generic;
using System.Linq;
using System;

namespace Interception.Core.Info
{
    public class StrictInterceptionInfo
    {
        // public string[] IgnoreCallerAssemblies { get; set; }
        public TypeInfo Interceptor { get; set; }
        public TargetMethod Target { get; set; }
        public string AssemblyPath { get; set; }
    }

    public class LoaderInfo
    {
        // public string[] IgnoreCallerAssemblies { get; set; }
        public string TypeName { get; set; }
        public string AssemblyPath { get; set; }
        //public TargetMethod Target { get; set; }
    }

    //public class MethodFinderInfo
    //{
    //    public TypeInfo MethodFinder { get; set; }
    //    public TargetMethod Target { get; set; }
    //}

    public class TypeInfo
    {
        public string AssemblyName { get; set; }
        public string TypeName { get; set; }
    }

    public class TargetMethod
    {
        public string AssemblyName { get; set; }
        public string MethodName { get; set; }
        public int MethodParametersCount { get; set; }
        public string TypeName { get; set; }
    }

    //public class AttributedInterceptor
    //{
    //    public string AttributeType { get; set; }
    //    public TypeInfo Interceptor { get; set; }
    //}

    public class ProfilerInfo
    {
        public List<string> Assemblies { get; }
        public LoaderInfo Loader { get; }
        public List<string> SkipAssemblies { get; }
        public List<StrictInterceptionInfo> Strict { get; }

        public ProfilerInfo(List<string> assemblies, LoaderInfo loader, List<string> skipAssemblies, List<StrictInterceptionInfo> strict)
        {
            Assemblies = assemblies;
            Loader = loader;
            SkipAssemblies = skipAssemblies;
            Strict = strict;
        }
    }
}
