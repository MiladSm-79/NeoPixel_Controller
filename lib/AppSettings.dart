import 'package:flutter_secure_storage/flutter_secure_storage.dart';
class AppStorage {
  static final AppStorage instance = AppStorage._internal();
  factory AppStorage() => instance;
  AppStorage._internal();

  final _storage = const FlutterSecureStorage();

  String ip = "192.168.1.100";
  String port = "80";
  double ntpOffsetHours = 3.5;
  int pixelCount = 48;

  int r = 0;
  int g = 255;
  int b = 0;
  bool timerEnabled = true;
  int sleepStart = 3;
  int sleepEnd = 13;

  Future<void> load() async {
    ip = (await _storage.read(key: 'ip')) ?? ip;
    port = (await _storage.read(key: 'port')) ?? port;
    ntpOffsetHours =
        double.tryParse((await _storage.read(key: 'ntpOffsetHours')) ?? '') ??
        ntpOffsetHours;
    pixelCount =
        int.tryParse((await _storage.read(key: 'pixelCount')) ?? '') ??
        pixelCount;
    r = int.tryParse((await _storage.read(key: 'r')) ?? '') ?? r;
    g = int.tryParse((await _storage.read(key: 'g')) ?? '') ?? g;
    b = int.tryParse((await _storage.read(key: 'b')) ?? '') ?? b;
   timerEnabled =
    ((await _storage.read(key: 'timerEnabled'))?.toLowerCase() == 'true')
        ? true
        : timerEnabled;
    sleepStart =
        int.tryParse((await _storage.read(key: 'sleepStart')) ?? '') ??
        sleepStart;
    sleepEnd =
        int.tryParse((await _storage.read(key: 'sleepEnd')) ?? '') ?? sleepEnd;
  }

  Future<void> save() async {
    await _storage.write(key: 'ip', value: ip);
    await _storage.write(key: 'port', value: port);
    await _storage.write(
      key: 'ntpOffsetHours',
      value: ntpOffsetHours.toString(),
    );
    await _storage.write(key: 'pixelCount', value: pixelCount.toString());
    await _storage.write(key: 'r', value: r.toString());
    await _storage.write(key: 'g', value: g.toString());
    await _storage.write(key: 'b', value: b.toString());
    await _storage.write(key: 'timerEnabled', value: timerEnabled.toString());
    await _storage.write(key: 'sleepStart', value: sleepStart.toString());
    await _storage.write(key: 'sleepEnd', value: sleepEnd.toString());
  }
}