using MassTransit;
using Microsoft.AspNetCore.Mvc;
using System.Threading.Tasks;
using app.MessageBus;

namespace app.Controllers
{
    [Route("api/[controller]")]
  [ApiController]
  public class SampleController : ControllerBase
  {
    private readonly IBusControl _busControl;

    public SampleController(IBusControl busControl)
    {
        _busControl = busControl;
    }

    [HttpGet("good")]
    public async Task<ActionResult<string>> Good()
    {
      await _busControl.Publish(new MyMessage());
      return Ok();
    }

    [HttpGet("bad")]
    public async Task<ActionResult<string>> Bad()
    {
      await _busControl.Publish(new MyBadMessage());
      return Ok();
    }
  }
}
