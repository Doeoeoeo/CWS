#include "UI/CWSPlayerHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UCWSPlayerHUDWidget::SetHealth(
    float CurrentHealth,
    float MaxHealth
)
{
	const float HealthPercent = MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;

	HealthBar->SetPercent(
		FMath::Clamp(HealthPercent, 0.0f, 1.0f)
	);

	HealthText->SetText(
		FText::Format(
		    NSLOCTEXT(
		        "CWS",
		        "PlayerHealthFormat",
		        "{0} / {1}"
			),
		    FText::AsNumber(FMath::RoundToInt(CurrentHealth)),
		    FText::AsNumber(FMath::RoundToInt(MaxHealth))
		)
	);
}