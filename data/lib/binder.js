class Binder {
  _ko;
  _formElementsBindeados = [];
  _viewModel;
  $;

  constructor(viewModel, ko, $) {
    this._ko = ko;
    this._viewModel = viewModel;
    this.$ = $;
  }

  bindKnockOut() {
    let self = this;

    self._ko.bindingHandlers.textWithInit = {
      init: function(element, valueAccessor, allBindingsAccessor, viewModel) {
        var value = valueAccessor();

        if (!ko.isObservable(viewModel[value])) {
          viewModel[value] = self._ko.observable(viewModel[value]);
        }
      },
      update: function(element, valueAccessor, allBindingsAccessor, viewModel) {
        self._ko.bindingHandlers.text.update(element, function() {
          return viewModel[valueAccessor()];
        });
      }
    };

    Date.prototype.formatMMDDYYYY = function() {
      return (
        this.getFullYear() +
        "/" +
        this.getMonth() +
        1 +
        "/" +
        this.getDate() +
        " " +
        this.getHours() +
        ":" +
        this.getMinutes() +
        ":" +
        this.getSeconds()
      );
    };

    self._ko.bindingHandlers.date = {
      update: function(element, valueAccessor, allBindingsAccessor, viewModel) {
        var date = valueAccessor();
        var strDate = date.formatMMDDYYYY();
        if (self.$(element).is(":input")) $(element).val(strDate);
        else self.$(element).text(strDate);
      }
    };

    this._viewModel.lastResponse.subscribe(obj => {
      this.bindForms();
    });
  }

  bindForms() {
    let self = this;

    self._formElementsBindeados.forEach(formElementBindeado => {
      formElementBindeado.formElement.removeEventListener(
        "submit",
        formElementBindeado.handler
      );
    });
    self._formElementsBindeados = [];

    let formElements = document.querySelectorAll("form");
    formElements.forEach(formElement => {
      let handler = event => {
        event.preventDefault();
        let formData = new FormData(formElement);
        let model = Object.fromEntries(formData);
        formElement.reset();
        self._viewModel.sentData(model);
      };
      formElement.addEventListener("submit", handler);
      self._formElementsBindeados.push({ formElement, handler });
    });
  }
}

define(["knockout", "viewModel", "jquery"], (ko, viewModel, jquery) =>
  new Binder(viewModel, ko, jquery));
