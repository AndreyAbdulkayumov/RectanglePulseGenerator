#include "UI.h"


/***************************************************/
//
// Private variables
//
/***************************************************/

void (*Display_WriteString) (uint8_t X, uint8_t Y, char* String);

void (*Display_WriteNumber) (uint8_t X, uint8_t Y,
		DisplayedNumber* Number,
		const char* Format);

void (*Display_SetCursor) (uint8_t X, uint8_t Y);


struct FieldValue Amplitude_Digit_1;
struct FieldValue Amplitude_Digit_2;
struct FieldValue Amplitude_Digit_3;
struct FieldValue Amplitude_Digit_4;

struct FieldValue Period_Digit_1;
struct FieldValue Period_Digit_2;
struct FieldValue Period_Digit_3;
struct FieldValue Period_Digit_4;
struct FieldValue Period_Digit_5;
struct FieldValue Period_Digit_6;
struct FieldValue Period_Digit_7;

struct FieldValue DutyCycle_Digit_1;
struct FieldValue DutyCycle_Digit_2;
struct FieldValue DutyCycle_Digit_3;
struct FieldValue DutyCycle_Digit_4;
struct FieldValue DutyCycle_Digit_5;
struct FieldValue DutyCycle_Digit_6;
struct FieldValue DutyCycle_Digit_7;


struct FieldValue* UI_AllFields[UI_NumberOfFields] =
{
		&Period_Digit_1,
		&Period_Digit_2, &Period_Digit_3, &Period_Digit_4,
		&Period_Digit_5, &Period_Digit_6, &Period_Digit_7,

		&DutyCycle_Digit_1,
		&DutyCycle_Digit_2, &DutyCycle_Digit_3, &DutyCycle_Digit_4,
		&DutyCycle_Digit_5, &DutyCycle_Digit_6, &DutyCycle_Digit_7,

		&Amplitude_Digit_1,
		&Amplitude_Digit_2, &Amplitude_Digit_3, &Amplitude_Digit_4
};


/***************************************************/
//
// Public variables
//
// С ними может может взаимодействовать внешний код
// через ключевое слово extern
//
/***************************************************/

struct MultiDigitNumber Period =
{
		.X = 9,
		.Y = 1,

		.AllFields = {
				&Period_Digit_1,
				&Period_Digit_2, &Period_Digit_3, &Period_Digit_4,
				&Period_Digit_5, &Period_Digit_6, &Period_Digit_7
		},

		.AmountOfDigits = 7,

		.Value = 100000,
		.MaxValue = 1000000    // us
};

struct MultiDigitNumber DutyCycle =
{
		.X = 9,
		.Y = 2,

		.AllFields = {
				&DutyCycle_Digit_1,
				&DutyCycle_Digit_2, &DutyCycle_Digit_3, &DutyCycle_Digit_4,
				&DutyCycle_Digit_5, &DutyCycle_Digit_6, &DutyCycle_Digit_7
		},

		.AmountOfDigits = 7,

		.Value = 458,
		.MaxValue = 1000000    // us
};

struct MultiDigitNumber Amplitude =
{
		.X = 13,
		.Y = 3,

		.AllFields = {
				&Amplitude_Digit_1,
				&Amplitude_Digit_2, &Amplitude_Digit_3, &Amplitude_Digit_4
		},

		.AmountOfDigits = 4,

		.Value = 2000,
		// .MaxValue инициализировано в функции UI_Init()
};


struct MultiDigitNumber* SelectedVariable;
struct FieldValue* SelectedField;


/*********************************************/
//
//	Private functions
//
/*********************************************/

void InitMultiDigitNumber(struct MultiDigitNumber Value)
{
	uint8_t Position_X_Offset = 0;

	double SplitDigits[Value.AmountOfDigits];

	for (int i = 0; i < Value.AmountOfDigits; i++)
	{
		SplitDigits[i] = (int)(((int)Value.Value % (int)pow(10, (i + 1))) / pow(10, i));
	}

	for (int i = 0; i < Value.AmountOfDigits; i++)
	{
		*Value.AllFields[i] = (struct FieldValue)
		{
				.X = Value.X + Position_X_Offset + i,
				.Y = Value.Y,

				.Number =
				{
						.Value = SplitDigits[Value.AmountOfDigits - 1 - i],
						.DisplayedValue = -1,
						.DisplayedValueLength = 1
				},

				.MaxValue = 9,

				.ValueMultiplier = 1,

				.Additive = 1,

				.Format = "%.0f"
		};

		Display_WriteNumber(
				Value.AllFields[i]->X,
				Value.AllFields[i]->Y,
				&Value.AllFields[i]->Number,
				Value.AllFields[i]->Format);

		if ((i == 0 && Value.AmountOfDigits != 1) ||
			(i == 3 && Value.AmountOfDigits != 4))
		{
			Display_WriteString(
					Value.AllFields[i]->X + 1,
					Value.AllFields[i]->Y,
					".");

			Position_X_Offset++;
		}
	}
}


double CalculateValue(struct MultiDigitNumber* Number)
{
	double Value = 0;

	for (int i = 0; i < Number->AmountOfDigits; i++)
	{
		Value += Number->AllFields[i]->Number.Value *
				pow(10, Number->AmountOfDigits - 1 - i);
	}

	return Value;
}


/*********************************************/
//
//	Public functions
//
/*********************************************/

void UI_Init(double Vref,
		void (*Function_WriteString) (uint8_t X, uint8_t Y, char* String),
		void (*Function_WriteNumber) (uint8_t X, uint8_t Y,
				DisplayedNumber* Number,
				const char* Format),
		void (*Function_SetCursor) (uint8_t X, uint8_t Y))
{
	Amplitude.MaxValue = Vref * 1000;	// mV

	Display_WriteString = Function_WriteString;
	Display_WriteNumber = Function_WriteNumber;
	Display_SetCursor = Function_SetCursor;

	/**********************************/
	//
	// Period
	//
	/**********************************/

	Display_WriteString(1, 1, "Period");

	InitMultiDigitNumber(Period);

	Display_WriteString(19, 1, "us");

	/**********************************/
	//
	// DutyCycle
	//
	/**********************************/

	Display_WriteString(1, 2, "DuCycl");

	InitMultiDigitNumber(DutyCycle);

	Display_WriteString(19, 2, "us");

	/**********************************/
	//
	// Amplitude
	//
	/**********************************/

	Display_WriteString(1, 3, "Amplitude");

	InitMultiDigitNumber(Amplitude);

	Display_WriteString(19, 3, "mV");


	SelectedVariable = &Period;
	SelectedField = UI_AllFields[0];

	Display_SetCursor(
			SelectedField->X,
			SelectedField->Y);
}


UpdateUI_Status UI_DisplayUpdate(uint32_t Value)
{
	double CalculatingValue = 0;

	double OldValue = SelectedField->Number.Value;

	SelectedField->Number.Value = Value * SelectedField->Additive;

	CalculatingValue = CalculateValue(SelectedVariable);

	if (CalculatingValue > SelectedVariable->MaxValue)
	{
		SelectedField->Number.Value = OldValue;

		return ValueOverflow;
	}

	else
	{
		SelectedVariable->Value = CalculatingValue;

		Display_WriteNumber(SelectedField->X, SelectedField->Y,
				&SelectedField->Number, SelectedField->Format);

		Display_SetCursor(
				SelectedField->X,
				SelectedField->Y);

		return OK;
	}
}


struct FieldValue* UI_GetField(uint8_t FieldNumber)
{
	return UI_AllFields[FieldNumber - 1];
}


void UI_ChangeField(uint8_t FieldNumber)
{
	if (FieldNumber >= 1 && FieldNumber <= 7)
	{
		SelectedVariable = &Period;
	}

	else if (FieldNumber >= 8 && FieldNumber <= 14)
	{
		SelectedVariable = &DutyCycle;
	}

	else
	{
		SelectedVariable = &Amplitude;
	}

	SelectedField = UI_AllFields[FieldNumber - 1];

	Display_SetCursor(
			SelectedField->X,
			SelectedField->Y);
}
