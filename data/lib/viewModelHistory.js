class ViewModelHistory {
  ko;
  constructor(ko) {
    this.ko = ko;
    this.socketResponses = ko.observableArray();
    this.sentDataHistory = ko.observableArray();
    this.errors = ko.observableArray();
  }

  subscribe(viewModel) {
    let self = this;
    viewModel.lastResponse.subscribe(r => self.socketResponses.push(r));
    viewModel.sentData.subscribe(d => self.sentDataHistory.push(d));
    viewModel.error.subscribe(e => self.errors.push(e));

    viewModel.history = self.ko.observable(self);
  }
}

define(["knockout"], ko => {
  let history = new ViewModelHistory(ko);
  history.subscribe(viewModel);
  return history;
});
