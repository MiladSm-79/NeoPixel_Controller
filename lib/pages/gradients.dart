// gradient_palette_page.dart
import 'package:flutter/material.dart';
import 'package:flutter_colorpicker/flutter_colorpicker.dart';
import 'package:http/http.dart' as http;
import '../theme/Colors.dart';
import '../theme/fonts.dart';
import '../AppSettings.dart';

class ColorStop {
  final int id;
  Color color;
  double endPosition; // 0.0 to 1.0 (END of this segment)

  ColorStop({required this.id, required this.color, required this.endPosition});
}

class GradientPalettePage extends StatefulWidget {
  const GradientPalettePage({super.key});

  @override
  State<GradientPalettePage> createState() => _GradientPalettePageState();
}

class _GradientPalettePageState extends State<GradientPalettePage> {
  static const int _maxStops = 10;
  late List<ColorStop> _stops;

  @override
  void initState() {
    super.initState();
    _stops = [
      ColorStop(id: 0, color: Colors.red, endPosition: 0.5),
      ColorStop(id: 1, color: Colors.black, endPosition: 1.0),
    ];
  }

  @override
  Widget build(BuildContext context) {
    final s = AppStorage.instance;
    final pixelCount = s.pixelCount;

    return Scaffold(
      appBar: AppBar(
        title: const Text("Gradient Palette"),
        backgroundColor: ProjectColors.primaryColor,
        foregroundColor: Colors.white,
      ),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          // PREVIEW
          _buildPreview(pixelCount),
          const SizedBox(height: 24),

          // COLOR STOPS
          ..._buildStopTiles(),
          const SizedBox(height: 16),

          if (_stops.length < _maxStops) _buildAddButton(),
          const SizedBox(height: 24),

          _buildApplyButton(s),
        ],
      ),
    );
  }

  // ─────────────────────────────────────────────────────────────────────
  // PREVIEW
  // ─────────────────────────────────────────────────────────────────────
  Widget _buildPreview(int pixelCount) {
    final colors = _interpolate(pixelCount);
    final sorted = [..._stops]..sort((a, b) => a.endPosition.compareTo(b.endPosition));
    final gradientColors = sorted.map((s) => s.color).toList();
    final gradientStops = sorted.map((s) => s.endPosition).toList();

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text("Preview ($pixelCount LEDs)", style: Fonts.smallTextStyle),
        const SizedBox(height: 8),

        // Smooth Gradient Bar
        Container(
          height: 50,
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(8),
            border: Border.all(color: Colors.grey.shade400),
            gradient: LinearGradient(
              colors: gradientColors,
              stops: gradientStops,
            ),
          ),
        ),
        const SizedBox(height: 8),

        // Pixel LED Preview
        Container(
          height: 20,
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(8),
            border: Border.all(color: Colors.grey.shade400),
          ),
          child: ClipRRect(
            borderRadius: BorderRadius.circular(8),
            child: Row(
              children: colors.isEmpty
                  ? [Container(color: Colors.grey[300])]
                  : colors
                      .map((c) => Expanded(
                            child: ColoredBox(color: c),
                          ))
                      .toList(),
            ),
          ),
        ),
      ],
    );
  }

  // ─────────────────────────────────────────────────────────────────────
  // COLOR STOP TILES
  // ─────────────────────────────────────────────────────────────────────
  List<Widget> _buildStopTiles() {
    final sorted = [..._stops]..sort((a, b) => a.endPosition.compareTo(b.endPosition));
    final widgets = <Widget>[];

    for (int i = 0; i < sorted.length; i++) {
      final stop = sorted[i];
      final isFirst = i == 0;
      final isLast = i == sorted.length - 1;

      final double prevEnd = isFirst ? 0.0 : sorted[i - 1].endPosition;
      final double nextEnd = isLast ? 1.0 : sorted[i + 1].endPosition;

      final double minPct = isFirst ? 0.0 : (prevEnd * 100) + 1.0;
      final double maxPct = isLast ? 100.0 : (nextEnd * 100) - 1.0;

      final double safeMin = minPct.clamp(0.0, 100.0);
      final double safeMax = maxPct.clamp(safeMin, 100.0);
      final double currentValue = (stop.endPosition * 100).clamp(safeMin, safeMax);

      widgets.add(
        Card(
          key: ValueKey(stop.id),
          margin: const EdgeInsets.only(bottom: 12),
          child: Padding(
            padding: const EdgeInsets.all(12),
            child: Row(
              children: [
                // COLOR BOX
                GestureDetector(
                  onTap: () => _pickColor(stop),
                  child: Container(
                    width: 40,
                    height: 40,
                    decoration: BoxDecoration(
                      color: stop.color,
                      borderRadius: BorderRadius.circular(8),
                      border: Border.all(color: Colors.grey.shade300),
                    ),
                  ),
                ),
                const SizedBox(width: 12),

                // LABEL + SLIDER
                Expanded(
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        "${(prevEnd * 100).round()}% → ${currentValue.round()}%",
                        style: Fonts.smallTextStyle.copyWith(fontWeight: FontWeight.bold),
                      ),
                      Slider(
                        value: currentValue,
                        min: safeMin,
                        max: safeMax,
                        divisions: 100,
                        label: '${currentValue.round()}%',
                        onChanged: isLast
                            ? null
                            : (v) {
                                setState(() {
                                  final originalStop = _stops.firstWhere((s) => s.id == stop.id);
                                  originalStop.endPosition = v / 100;

                                  // Auto-push next stop
                                  if (!isLast && i + 1 < sorted.length) {
                                    final nextStop = _stops.firstWhere((s) => s.id == sorted[i + 1].id);
                                    final minNext = originalStop.endPosition + 0.01;
                                    if (nextStop.endPosition < minNext) {
                                      nextStop.endPosition = minNext;
                                    }
                                  }
                                });
                              },
                      ),
                    ],
                  ),
                ),

                // DELETE BUTTON
                if (!isFirst && !isLast && _stops.length > 2)
                  IconButton(
                    icon: const Icon(Icons.delete, color: Colors.red),
                    onPressed: () => setState(() => _stops.removeWhere((s) => s.id == stop.id)),
                  ),
              ],
            ),
          ),
        ),
      );
    }
    return widgets;
  }

  // ─────────────────────────────────────────────────────────────────────
  // ADD BUTTON
  // ─────────────────────────────────────────────────────────────────────
  Widget _buildAddButton() {
    return TextButton.icon(
      onPressed: _addStop,
      icon: const Icon(Icons.add),
      label: const Text("Add Color Stop"),
    );
  }

  void _addStop() {
    if (_stops.length >= _maxStops) return;

    final sorted = [..._stops]..sort((a, b) => a.endPosition.compareTo(b.endPosition));
    final lastEnd = sorted.last.endPosition;
    final newEnd = (lastEnd + 1.0) / 2;

    setState(() {
      _stops.add(ColorStop(
        id: _stops.length,
        color: Colors.white,
        endPosition: newEnd.clamp(0.1, 0.9),
      ));
    });
  }

  // ─────────────────────────────────────────────────────────────────────
  // COLOR PICKER
  // ─────────────────────────────────────────────────────────────────────
  void _pickColor(ColorStop stop) {
    showDialog(
      context: context,
      builder: (_) => AlertDialog(
        title: const Text("Pick Color"),
        content: BlockPicker(
          pickerColor: stop.color,
          onColorChanged: (c) {
            setState(() => stop.color = c);
            Navigator.pop(context);
          },
        ),
      ),
    );
  }

  // ─────────────────────────────────────────────────────────────────────
  // APPLY BUTTON
  // ─────────────────────────────────────────────────────────────────────
  Widget _buildApplyButton(AppStorage s) {
    return ElevatedButton.icon(
      onPressed: () => _apply(s),
      icon: const Icon(Icons.send),
      label: const Text("Apply to LEDs"),
      style: ElevatedButton.styleFrom(
        padding: const EdgeInsets.symmetric(vertical: 16),
        backgroundColor: ProjectColors.buttonColor,
      ),
    );
  }

  // ─────────────────────────────────────────────────────────────────────
  // INTERPOLATION (FIXED: TRUE BLACK)
  // ─────────────────────────────────────────────────────────────────────
  List<Color> _interpolate(int pixelCount) {
    if (pixelCount <= 0 || _stops.length < 2) {
      return List.filled(pixelCount, Colors.grey);
    }

    final sortedStops = [..._stops]..sort((a, b) => a.endPosition.compareTo(b.endPosition));
    final pixels = <Color>[];

    for (int i = 0; i < pixelCount; i++) {
      final t = i / (pixelCount - 1).toDouble();
      Color col = sortedStops.last.color;

      for (int s = 0; s < sortedStops.length; s++) {
        final start = s == 0 ? 0.0 : sortedStops[s - 1].endPosition;
        final end = sortedStops[s].endPosition;

        if (t >= start && t <= end) {
          final localT = (t - start) / (end - start);
          final colorA = s == 0 ? sortedStops[0].color : sortedStops[s - 1].color;
          final colorB = sortedStops[s].color;

          // LERP WITH TRUE BLACK
          final r = (colorA.red + (colorB.red - colorA.red) * localT).round();
          final g = (colorA.green + (colorB.green - colorA.green) * localT).round();
          final b = (colorA.blue + (colorB.blue - colorA.blue) * localT).round();

          // FORCE TRUE BLACK: if all < 5 → 0
          final finalR = r < 5 ? 0 : r;
          final finalG = g < 5 ? 0 : g;
          final finalB = b < 5 ? 0 : b;

          col = Color.fromARGB(255, finalR, finalG, finalB);
          break;
        }
      }
      pixels.add(col);
    }
    return pixels;
  }

  // ─────────────────────────────────────────────────────────────────────
  // HTTP APPLY (NO ENCODING)
  // ─────────────────────────────────────────────────────────────────────
  Future<void> _apply(AppStorage s) async {
    final pixels = _interpolate(s.pixelCount);
    final rgb = pixels.map((c) => '${c.red},${c.green},${c.blue}').join('|');

    final baseUrl = 'http://${s.ip}:${s.port}/gradient';
    final query = 'colors=$rgb&pixel_count=${s.pixelCount}';
    final fullUrl = '$baseUrl?$query';

    print('Sending RAW URL:');
    print(fullUrl);
    print('URL Length: ${fullUrl.length}');
    print('First 3 colors: ${pixels.take(3).map((c) => '${c.red},${c.green},${c.blue}').join(' | ')}');

    try {
      final uri = Uri.parse(fullUrl);
      final resp = await http.get(uri).timeout(const Duration(seconds: 3));

      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Success: ${resp.body}')),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Error: $e')),
      );
    }
  }
}