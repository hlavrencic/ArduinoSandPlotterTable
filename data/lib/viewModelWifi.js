class ViewModelWifi {
  ko;

  constructor(ko) {
    let self = this;

    self.ko = ko;

    self.status = ko.observable();
    self.ip = ko.observable();
    self.networks = ko.observableArray();
  }

  subscribe(viewModel) {
    let self = this;

    viewModel.lastResponse.subscribe(response => {
      if (response.ip) {
        self.ip(response.ip);
      }

      if (response.wifiStatus) {
        self.status(response.wifiStatus);
      }

      if (response.scan) {
        self.networks(response.scan);
      }
    });

    viewModel.wifi = self.ko.observable(self);
  }
}

define(["knockout", "viewModel"], (knockout, viewModel) => {
  let model = new ViewModelWifi(knockout);
  model.subscribe(viewModel);
  return model;
});
