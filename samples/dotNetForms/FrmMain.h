#pragma once

namespace VExDebuggerform {

	void __stdcall InitForm( );



	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;

	/// <summary>
	/// Sumário para FrmMain
	/// </summary>
	public ref class FrmMain : public System::Windows::Forms::Form
	{
	public:
		FrmMain(void)
		{
			InitializeComponent();
			//
			//TODO: Adicione o código do construtor aqui
			//
		}

	protected:
		/// <summary>
		/// Limpar os recursos que estão sendo usados.
		/// </summary>
		~FrmMain()
		{
			if (components)
			{
				delete components;
			}
		}
	public: System::Windows::Forms::Button^ BtnAdd;
	protected:

	public: System::Windows::Forms::ListBox^ ExcpList1;
	public: System::Windows::Forms::ListBox^ ExcpList2;
	public: System::Windows::Forms::ListBox^ ExcpList3;
	public: System::Windows::Forms::ListBox^ ExcpList4;
	public: System::Windows::Forms::TextBox^ TbAddress;









	public: System::Windows::Forms::ComboBox^ CbType;


	public: System::Windows::Forms::ComboBox^ CbSize;

	public: System::Windows::Forms::Timer^ timer1;
	public: System::Windows::Forms::TabControl^ tab_page;
	public: System::Windows::Forms::TabPage^ tp_1;
	public: System::Windows::Forms::TabPage^ tp_2;
	public: System::Windows::Forms::TabPage^ tp_3;
	public: System::Windows::Forms::TabPage^ tp_4;
	public: System::Windows::Forms::Button^ BtnSave;

	public: System::Windows::Forms::SaveFileDialog^ saveFileDialog1;
	private: System::ComponentModel::IContainer^ components;

	protected:

	private:
		/// <summary>
		/// Variável de designer necessária.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Método necessário para suporte ao Designer - não modifique 
		/// o conteúdo deste método com o editor de código.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = ( gcnew System::ComponentModel::Container( ) );
			this->BtnAdd = ( gcnew System::Windows::Forms::Button( ) );
			this->ExcpList1 = ( gcnew System::Windows::Forms::ListBox( ) );
			this->ExcpList2 = ( gcnew System::Windows::Forms::ListBox( ) );
			this->ExcpList3 = ( gcnew System::Windows::Forms::ListBox( ) );
			this->ExcpList4 = ( gcnew System::Windows::Forms::ListBox( ) );
			this->TbAddress = ( gcnew System::Windows::Forms::TextBox( ) );
			this->CbType = ( gcnew System::Windows::Forms::ComboBox( ) );
			this->CbSize = ( gcnew System::Windows::Forms::ComboBox( ) );
			this->timer1 = ( gcnew System::Windows::Forms::Timer( this->components ) );
			this->tab_page = ( gcnew System::Windows::Forms::TabControl( ) );
			this->tp_1 = ( gcnew System::Windows::Forms::TabPage( ) );
			this->tp_2 = ( gcnew System::Windows::Forms::TabPage( ) );
			this->tp_3 = ( gcnew System::Windows::Forms::TabPage( ) );
			this->tp_4 = ( gcnew System::Windows::Forms::TabPage( ) );
			this->BtnSave = ( gcnew System::Windows::Forms::Button( ) );
			this->saveFileDialog1 = ( gcnew System::Windows::Forms::SaveFileDialog( ) );
			this->tab_page->SuspendLayout( );
			this->tp_1->SuspendLayout( );
			this->tp_2->SuspendLayout( );
			this->tp_3->SuspendLayout( );
			this->tp_4->SuspendLayout( );
			this->SuspendLayout( );
			// 
			// BtnAdd
			// 
			this->BtnAdd->Location = System::Drawing::Point( 382, 561 );
			this->BtnAdd->Name = L"BtnAdd";
			this->BtnAdd->Size = System::Drawing::Size( 85, 27 );
			this->BtnAdd->TabIndex = 0;
			this->BtnAdd->Text = L"Add";
			this->BtnAdd->UseVisualStyleBackColor = true;
			this->BtnAdd->Click += gcnew System::EventHandler( this, &FrmMain::BtnAdd_Click );
			// 
			// ExcpList1
			// 
			this->ExcpList1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->ExcpList1->Font = ( gcnew System::Drawing::Font( L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>( 0 ) ) );
			this->ExcpList1->FormattingEnabled = true;
			this->ExcpList1->ItemHeight = 17;
			this->ExcpList1->Location = System::Drawing::Point( 3, 3 );
			this->ExcpList1->Name = L"ExcpList1";
			this->ExcpList1->Size = System::Drawing::Size( 439, 508 );
			this->ExcpList1->TabIndex = 1;
			// 
			// ExcpList2
			// 
			this->ExcpList2->Dock = System::Windows::Forms::DockStyle::Fill;
			this->ExcpList2->FormattingEnabled = true;
			this->ExcpList2->ItemHeight = 15;
			this->ExcpList2->Location = System::Drawing::Point( 3, 3 );
			this->ExcpList2->Name = L"ExcpList2";
			this->ExcpList2->Size = System::Drawing::Size( 439, 508 );
			this->ExcpList2->TabIndex = 1;
			// 
			// ExcpList3
			// 
			this->ExcpList3->Dock = System::Windows::Forms::DockStyle::Fill;
			this->ExcpList3->FormattingEnabled = true;
			this->ExcpList3->ItemHeight = 15;
			this->ExcpList3->Location = System::Drawing::Point( 3, 3 );
			this->ExcpList3->Name = L"ExcpList3";
			this->ExcpList3->Size = System::Drawing::Size( 439, 508 );
			this->ExcpList3->TabIndex = 1;
			// 
			// ExcpList4
			// 
			this->ExcpList4->Dock = System::Windows::Forms::DockStyle::Fill;
			this->ExcpList4->FormattingEnabled = true;
			this->ExcpList4->ItemHeight = 15;
			this->ExcpList4->Location = System::Drawing::Point( 3, 3 );
			this->ExcpList4->Name = L"ExcpList4";
			this->ExcpList4->Size = System::Drawing::Size( 439, 508 );
			this->ExcpList4->TabIndex = 1;
			// 
			// TbAddress
			// 
			this->TbAddress->Location = System::Drawing::Point( 14, 564 );
			this->TbAddress->Name = L"TbAddress";
			this->TbAddress->Size = System::Drawing::Size( 120, 23 );
			this->TbAddress->TabIndex = 6;
			// 
			// CbType
			// 
			this->CbType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->CbType->FormattingEnabled = true;
			this->CbType->Items->AddRange( gcnew cli::array< System::Object^  >( 3 ) { L"Execute", L"Read/Write", L"Read" } );
			this->CbType->Location = System::Drawing::Point( 142, 564 );
			this->CbType->Name = L"CbType";
			this->CbType->Size = System::Drawing::Size( 114, 23 );
			this->CbType->TabIndex = 7;
			// 
			// CbSize
			// 
			this->CbSize->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->CbSize->FormattingEnabled = true;
			this->CbSize->Items->AddRange( gcnew cli::array< System::Object^  >( 4 ) { L"1 Byte", L"2 Byte", L"8 Byte", L"4 Byte" } );
			this->CbSize->Location = System::Drawing::Point( 262, 564 );
			this->CbSize->Name = L"CbSize";
			this->CbSize->Size = System::Drawing::Size( 114, 23 );
			this->CbSize->TabIndex = 8;
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 2000;
			this->timer1->Tick += gcnew System::EventHandler( this, &FrmMain::Timer1_Tick );
			// 
			// tab_page
			// 
			this->tab_page->Controls->Add( this->tp_1 );
			this->tab_page->Controls->Add( this->tp_2 );
			this->tab_page->Controls->Add( this->tp_3 );
			this->tab_page->Controls->Add( this->tp_4 );
			this->tab_page->Location = System::Drawing::Point( 14, 14 );
			this->tab_page->Name = L"tab_page";
			this->tab_page->SelectedIndex = 0;
			this->tab_page->Size = System::Drawing::Size( 453, 542 );
			this->tab_page->TabIndex = 9;
			// 
			// tp_1
			// 
			this->tp_1->Controls->Add( this->ExcpList1 );
			this->tp_1->Location = System::Drawing::Point( 4, 24 );
			this->tp_1->Name = L"tp_1";
			this->tp_1->Padding = System::Windows::Forms::Padding( 3 );
			this->tp_1->Size = System::Drawing::Size( 445, 514 );
			this->tp_1->TabIndex = 0;
			this->tp_1->Text = L"0";
			this->tp_1->UseVisualStyleBackColor = true;
			// 
			// tp_2
			// 
			this->tp_2->Controls->Add( this->ExcpList2 );
			this->tp_2->Location = System::Drawing::Point( 4, 24 );
			this->tp_2->Name = L"tp_2";
			this->tp_2->Padding = System::Windows::Forms::Padding( 3 );
			this->tp_2->Size = System::Drawing::Size( 445, 514 );
			this->tp_2->TabIndex = 1;
			this->tp_2->Text = L"0";
			this->tp_2->UseVisualStyleBackColor = true;
			// 
			// tp_3
			// 
			this->tp_3->Controls->Add( this->ExcpList3 );
			this->tp_3->Location = System::Drawing::Point( 4, 24 );
			this->tp_3->Name = L"tp_3";
			this->tp_3->Padding = System::Windows::Forms::Padding( 3 );
			this->tp_3->Size = System::Drawing::Size( 445, 514 );
			this->tp_3->TabIndex = 2;
			this->tp_3->Text = L"0";
			this->tp_3->UseVisualStyleBackColor = true;
			// 
			// tp_4
			// 
			this->tp_4->Controls->Add( this->ExcpList4 );
			this->tp_4->Location = System::Drawing::Point( 4, 24 );
			this->tp_4->Name = L"tp_4";
			this->tp_4->Padding = System::Windows::Forms::Padding( 3 );
			this->tp_4->Size = System::Drawing::Size( 445, 514 );
			this->tp_4->TabIndex = 3;
			this->tp_4->Text = L"0";
			this->tp_4->UseVisualStyleBackColor = true;
			// 
			// BtnSave
			// 
			this->BtnSave->Location = System::Drawing::Point( 14, 594 );
			this->BtnSave->Name = L"BtnSave";
			this->BtnSave->Size = System::Drawing::Size( 453, 31 );
			this->BtnSave->TabIndex = 10;
			this->BtnSave->Text = L"Save logs";
			this->BtnSave->UseVisualStyleBackColor = true;
			this->BtnSave->Click += gcnew System::EventHandler( this, &FrmMain::BtnSave_Click );
			// 
			// FrmMain
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF( 7, 15 );
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size( 481, 636 );
			this->Controls->Add( this->BtnSave );
			this->Controls->Add( this->tab_page );
			this->Controls->Add( this->CbSize );
			this->Controls->Add( this->CbType );
			this->Controls->Add( this->TbAddress );
			this->Controls->Add( this->BtnAdd );
			this->Font = ( gcnew System::Drawing::Font( L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>( 0 ) ) );
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->Name = L"FrmMain";
			this->Text = L"u";
			this->tab_page->ResumeLayout( false );
			this->tp_1->ResumeLayout( false );
			this->tp_2->ResumeLayout( false );
			this->tp_3->ResumeLayout( false );
			this->tp_4->ResumeLayout( false );
			this->ResumeLayout( false );
			this->PerformLayout( );

		}
#pragma endregion
	private: System::Void BtnAdd_Click(System::Object^ sender, System::EventArgs^ e);
	private: System::Void Timer1_Tick(System::Object^ sender, System::EventArgs^ e);
	private: System::Void BtnSave_Click( System::Object^ sender, System::EventArgs^ e );
};
}
