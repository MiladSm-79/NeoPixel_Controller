import 'package:flutter/material.dart';
import 'package:neo_pixel/theme/fonts.dart';
class SleepTimerControls extends StatelessWidget {
  final bool enabled;
  final double start;
  final double end;
  final ValueChanged<double> onStartChanged;
  final ValueChanged<double> onEndChanged;

  const SleepTimerControls({
    super.key,
    required this.enabled,
    required this.start,
    required this.end,
    required this.onStartChanged,
    required this.onEndChanged,
  });

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        _slider("Start Hour", start, onStartChanged),
        _slider("End Hour", end, onEndChanged),
      ],
    );
  }

  Widget _slider(String label, double value, ValueChanged<double> cb) {
    return Expanded(
      child: Column(
        children: [
          Text(label, style: Fonts.smallTextStyle),
          Slider(
            value: value,
            min: 0,
            max: 23,
            divisions: 23,
            label: "${value.toInt()}:00",
            onChanged: enabled ? cb : null,
          ),
        ],
      ),
    );
  }
}
