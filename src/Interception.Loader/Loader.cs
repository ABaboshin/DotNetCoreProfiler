using Interception.Attributes;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;

namespace Interception
{
    public class Loader
    {

        //List<string> _path = new List<string>();

        public Loader(string interceptionDlls)
        {
            Console.WriteLine($"Interception.Loader {interceptionDlls}");

            foreach (var dll in interceptionDlls.Split(new char[] { ',' }, StringSplitOptions.RemoveEmptyEntries))
            {
                //_path.Add(new FileInfo(dll).Directory.FullName);
                //AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;

                //var assembly = AppDomain.CurrentDomain.Load(/*File.ReadAllBytes(dll)*/AssemblyName.GetAssemblyName(dll));
                var assembly = Assembly.LoadFrom(dll);

                ProcessInitializer(assembly);
            }
        }

        //private Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        //{
        //    //var fp = Path.Combine(_path[0], args.RequestingAssembly.GetName().Name + ".dll");
        //    //if (File.Exists(fp))
        //    //{
        //    //    Console.WriteLine($"Load Assembly from {fp}");
        //    //    return Assembly.LoadFrom(fp);
        //    //}

        //    Console.WriteLine($"Resolve {args.RequestingAssembly.FullName}");
        //    return null;
        //}

        private void ProcessInitializer(Assembly assembly)
        {
            var initializers = assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(InitializeAttribute).FullName).Any())
                .ToList();

            foreach (var initializer in initializers)
            {
                Activator.CreateInstance(initializer);
            }
        }
    }
}
