import 'package:flutter/material.dart';
import 'package:flutter_colorpicker/flutter_colorpicker.dart';
import 'package:http/http.dart' as http;
import 'package:neo_pixel/AppSettings.dart';
import 'package:neo_pixel/pages/animations.dart';
import 'package:neo_pixel/pages/gradients.dart';
import 'package:neo_pixel/theme/Colors.dart';
import 'package:neo_pixel/theme/fonts.dart';
import 'package:neo_pixel/widgets/sleep_timer.dart';
import '../widgets/color_picker_section.dart';
import 'settings_page.dart';

class ControlPage extends StatefulWidget {
  const ControlPage({super.key});

  @override
  State<ControlPage> createState() => _ControlPageState();
}

class _ControlPageState extends State<ControlPage> {
  late Color currentColor;
  double r = 0, g = 255, b = 0;

  bool timerEnabled = true;
  double sleepStart = 3;
  double sleepEnd = 13;

  bool isSending = false;

  static const _httpTimeout = Duration(seconds: 1);

  @override
  void initState() {
    super.initState();
    final s = AppStorage.instance;
    r = s.r.toDouble();
    g = s.g.toDouble();
    b = s.b.toDouble();
    currentColor = Color.fromARGB(255, s.r, s.g, s.b);
    timerEnabled = s.timerEnabled;
    sleepStart = s.sleepStart.toDouble();
    sleepEnd = s.sleepEnd.toDouble();
  }

  void _onColorChanged(Color c) {
    currentColor = c;
    r = c.r * 255.toDouble();
    g = c.g * 255.toDouble();
    b = c.b * 255.toDouble();
  }

  // Called when user releases finger after picking color
  void _onColorChangeEnd() => setState(() {});

  Future<void> _applyAll() async {
    final s = AppStorage.instance;

    if (s.ip.isEmpty || s.port.isEmpty) {
      _showSnack('server address is missing or incorrect');
      return;
    }

    final params = {
      'r': r.toInt(),
      'g': g.toInt(),
      'b': b.toInt(),
      'enable': timerEnabled ? 1 : 0,
      'start': sleepStart.toInt(),
      'end': sleepEnd.toInt(),
      'ntp_offset_minutes': (s.ntpOffsetHours * 60).round(),
      'pixel_count': s.pixelCount,
    }.map((k, v) => MapEntry(k, v.toString()));

    final uri = Uri.http('${s.ip}:${s.port}', "/apply", params);

    setState(() => isSending = true);

    try {
      final resp = await http.get(uri).timeout(_httpTimeout);
      _showSnack(resp.body);
      await _saveSettings();
    } catch (e) {
      _showSnack("$e error !");
    } finally {
      if (mounted) setState(() => isSending = false);
    }
  }

  Future<void> _saveSettings() async {
    final s = AppStorage.instance;
    s
      ..r = r.toInt()
      ..g = g.toInt()
      ..b = b.toInt()
      ..timerEnabled = timerEnabled
      ..sleepStart = sleepStart.toInt()
      ..sleepEnd = sleepEnd.toInt();

    await s.save();
  }

  void _showSnack(String msg) {
    if (!mounted) return;
    ScaffoldMessenger.of(
      context,
    ).showSnackBar(SnackBar(content: Text(msg, style: Fonts.smallTextStyle)));
  }

  @override
  Widget build(BuildContext context) {
    final fg = useWhiteForeground(currentColor)
        ? ProjectColors.whiteAppbarColor
        : ProjectColors.blackAppbarColor;

    return Scaffold(
      appBar: AppBar(
        backgroundColor: currentColor,
        foregroundColor: fg,
        title: Text("NeoPixel Controller", style: Fonts.mediumTextStyle),
        actions: [
          IconButton(
            icon: const Icon(Icons.settings),
            onPressed: () async {
              await Navigator.push(
                context,
                MaterialPageRoute(builder: (_) => const SettingsPage()),
              );
              setState(() {});
            },
          ),
          IconButton(
            icon: const Icon(Icons.animation),
            onPressed: () async {
              await Navigator.push(
                context,
                MaterialPageRoute(builder: (_) => const Animations()),
              );
              setState(() {});
            },
          ),
        ],
      ),

      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          ColorPickerSection(
            pickerColor: currentColor,
            onChanged: _onColorChanged,
            onChangeEnd: _onColorChangeEnd,
          ),

          const SizedBox(height: 16),

          SwitchListTile(
            title: Text("Enable Sleep Timer", style: Fonts.smallTextStyle),
            value: timerEnabled,
            onChanged: (v) => setState(() => timerEnabled = v),
          ),

          SleepTimerControls(
            enabled: timerEnabled,
            start: sleepStart,
            end: sleepEnd,
            onStartChanged: (v) => setState(() => sleepStart = v),
            onEndChanged: (v) => setState(() => sleepEnd = v),
          ),

          const SizedBox(height: 30),

          Center(
            child: ElevatedButton.icon(
              onPressed: isSending ? null : _applyAll,
              icon: AnimatedSwitcher(
                duration: const Duration(milliseconds: 300),
                child: isSending
                    ? const SizedBox(
                        width: 20,
                        height: 20,
                        child: CircularProgressIndicator(strokeWidth: 2),
                      )
                    : const Icon(Icons.check, size: 20),
              ),
              label: Text(
                isSending ? "Applying..." : "Apply",
                style: Fonts.smallTextStyle,
              ),
              style: ElevatedButton.styleFrom(
                padding: const EdgeInsets.symmetric(
                  horizontal: 50,
                  vertical: 16,
                ),
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(24),
                ),
                backgroundColor: isSending
                    ? ProjectColors.buttonAnimationColor
                    : ProjectColors.buttonColor,
                foregroundColor: ProjectColors.defaultWhiteColor,
              ),
            ),
          ),
          GestureDetector(
            onTap: (){ 
              Navigator.push(
                context,
                MaterialPageRoute(builder: (_) => const GradientPalettePage()),
              );},
            child: Text("Gradients",style: TextStyle(fontSize: 24),),
          )
        ],
      ),
    );
  }
}
