namespace Interception.Core
{
    public interface IComposedInterceptor
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
        /// execute interceptor
        /// </summary>
        /// <returns></returns>
        object Execute();

        void AddChild(IInterceptor interceptor);
    }
}
