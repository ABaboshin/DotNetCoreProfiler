using System;
using System.Reflection;

namespace Interception.Core
{
    /// <summary>
    /// interceptor
    /// </summary>
    public interface IInterceptor
    {
        /// <summary>
        /// interception priority
        /// </summary>
        int Priority { get; }

        /// <summary>
        /// this of intercepted class
        /// </summary>
        object This { get; set; }

        MethodInfo Method { get; set; }

        /// <summary>
        /// result of execution
        /// </summary>
        object Result { get; set; }

        /// <summary>
        /// exception if any
        /// </summary>
        Exception Exception { get; set; }

        /// <summary>
        /// get paraemeter value
        /// </summary>
        /// <param name="num"></param>
        /// <returns></returns>
        object GetParameter(int num);

        /// <summary>
        /// update parameter value
        /// </summary>
        /// <param name="num"></param>
        /// <param name="value"></param>
        void ModifyParameter(int num, object value);

        bool IsParameterModified(int num);

        /// <summary>
        /// set method parameters
        /// </summary>
        /// <param name="parameters"></param>
        void SetParameters(object[] parameters);

        /// <summary>
        /// execute before
        /// </summary>
        void ExecuteBefore();

        /// <summary>
        /// execute after
        /// </summary>
        void ExecuteAfter();

        /// <summary>
        /// true if the execution has to be skiped
        /// </summary>
        /// <returns></returns>
        bool SkipExecution();
    }
}
