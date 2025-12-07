import { cn } from '../lib/utils';

interface ProgressBarProps {
  value: number;
  max?: number;
  className?: string;
  showPercentage?: boolean;
}

export function ProgressBar({
  value,
  max = 100,
  className,
  showPercentage = true,
}: ProgressBarProps) {
  const percentage = Math.min(100, (value / max) * 100);

  return (
    <div className={cn('space-y-2', className)}>
      <div className="w-full bg-secondary rounded-full h-2.5 overflow-hidden">
        <div
          className="bg-primary h-2.5 transition-all duration-300 ease-out"
          style={{ width: `${percentage}%` }}
        />
      </div>
      {showPercentage && (
        <p className="text-sm text-muted-foreground text-center">
          {percentage.toFixed(1)}%
        </p>
      )}
    </div>
  );
}
