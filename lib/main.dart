import 'package:flutter/material.dart';
import 'package:neo_pixel/AppSettings.dart';
import 'pages/control_page.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await AppStorage.instance.load();
  runApp(const NeoPixelApp());
}

class NeoPixelApp extends StatelessWidget {
  const NeoPixelApp({super.key});

  @override
  Widget build(BuildContext context) {
    return const MaterialApp(
      debugShowCheckedModeBanner: false,
      home: ControlPage(),
    );
  }
}
