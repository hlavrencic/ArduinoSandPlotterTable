class WifiScan {
   
  constructor(ko, blocker, apiService) {
    this.blocker = blocker;
    this.apiService = apiService;
    this.ko = ko;

    this.networks = ko.observableArray();
  }

  scan() {
    let self = this;
    self.blocker.loading();

    self.apiService
      .post("startScan")
      .then(() => {
        let contador = 0;
        let interval = setInterval(() => {
          contador++;
          let canceled = false;
          self.getScan(scan => {
            if (canceled) return;
            clearInterval(interval);
            self.blocker.off();
            self.networks(scan);
          });

          if (contador >= 2) {
            self.blocker.loading(() => {
              canceled = true;
              clearInterval(interval);
              self.blocker.off();
            });
          }
        }, 2000);
      })
      .catch(function(error) {
        self.blocker.on(error);
      });
  }

  getScan(callbackReady) {
    let self = this;

    self.apiService.post("getScan").then(rta => {
      if (rta.scan) callbackReady(rta.scan);
    });
  }
}

define(["knockout", "mods/blocker", "apiService"], (
  ko,
  blocker,
  apiService
) => {
  let viewModel = new WifiScan(ko, blocker, apiService);
  ko.components.register("wifiscan", {
    viewModel: { instance: viewModel },
    template: { require: "text!cmp/wifiScan.html" }
  });
});
