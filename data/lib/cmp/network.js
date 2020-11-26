define(["knockout", "apiService", "mods/blocker", "mods/wifiStatus"], (
  ko,
  apiService,
  blocker,
  wifiStatus
) => {
  class NetWorkComponent {
    constructor(ko, apiService, blocker, wifiStatus, params) {
      this.apiService = apiService;
      this.blocker = blocker;
      this.wifiStatus = wifiStatus;

      this.ssid = ko.observable(params.ssid);
      this.password = ko.observable();
    }

    connect() {
      let self = this;
      let data = {};
      data.ssid = self.ssid();
      data.password = self.password();

      self.apiService.post("connect", data).then(() => {
        let cancellationToken = {};
        self.wifiStatus
          .waitConnection(cancellationToken)
          .then(() => {
            self.blocker.off();
          })
          .catch(reason => {
            self.blocker.on(reason);
          });

        self.blocker.loading(() => {
          cancellationToken.cancel = true;
        });
      });

      self.blocker.loading();
    }
  }

  ko.components.register("network", {
    viewModel: {
      createViewModel: (params, componentInfo) => {
        let model = new NetWorkComponent(
          ko,
          apiService,
          blocker,
          wifiStatus,
          params
        );

        return model;
      }
    },
    template: { require: "text!cmp/network.html" }
  });
});
