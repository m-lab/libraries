part of mlab;

Future<Map> lookup(String tool, [String address_family : '']) {
  String url = 'https://mlab-ns.appspot.com/$tool?format=json';
  if (address_family != '')
    url += '&address_family=$address_family';

  Completer completer = new Completer();
  HttpRequest request = new HttpRequest.get(url, (HttpRequest req) {
      print('${req.responseText}');
      Map response = parse(req.responseText);
      // TODO: check for 404
      completer.complete(response);
  });
  return completer.future;
}
