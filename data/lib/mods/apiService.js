class ApiService {
  constructor(axios) {
    this.axios = axios;
  }

  post(uri, data) {
    let self = this;
    return self.axios.post('/' + uri, data).then(self.handle);
  }

  get(uri) {
    let self = this;
    return self.axios.get('/' + uri).then(self.handle);
  }

  handle(rta) {
    if (rta.status != 200) {
      throw rta;
    }

    return rta.data;
  }
}

define(["axios"], axios => {
  return new ApiService(axios);
});
