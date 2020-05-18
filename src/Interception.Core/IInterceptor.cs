using System;

namespace Interception.Core
{
    /// <summary>
    /// interceptor
    /// </summary>
    public interface IInterceptor
    {
        /// <summary>
        /// this of intercepted class
        /// </summary>
        object This { get; set; }

        /// <summary>
        /// metadata token of intercepted method
        /// </summary>
        int MdToken { get; set; }

        /// <summary>
        /// pointer to module version of intercepted method
        /// </summary>
        long ModuleVersionPtr { get; set; }

        /// <summary>
        /// result of execution
        /// </summary>
        object Result { get; set; }

        /// <summary>
        /// exception if any
        /// </summary>
        Exception Exception { get; set; }

        /// <summary>
        /// set argument number
        /// </summary>
        /// <param name="number"></param>
        /// <returns></returns>
        void SetArgumentCount(int number);

        /// <summary>
        /// add parameter of intercepted method
        /// </summary>
        /// <param name="num"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        void AddParameter(int num, object value);

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
        void UpdateParameter(int num, object value);

        /// <summary>
        /// execute interceptor
        /// </summary>
        /// <returns></returns>
        object Execute();

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

        void AddChild(IInterceptor interceptor);
    }
}
