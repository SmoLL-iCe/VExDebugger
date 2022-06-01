#pragma once

namespace VExDebugform {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;

	/// <summary>
	/// Sumário para form_main
	/// </summary>
	public ref class form_main : public System::Windows::Forms::Form
	{
	public:
		form_main(void)
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
		~form_main()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^ button1;
	private: System::Windows::Forms::ListBox^ exp_list_1;


	private: System::Windows::Forms::ListBox^ exp_list_2;

	private: System::Windows::Forms::ListBox^ exp_list_3;

	private: System::Windows::Forms::ListBox^ exp_list_4;
	private: System::Windows::Forms::TextBox^ tb_address;

	private: System::Windows::Forms::ComboBox^ cb_type;
	private: System::Windows::Forms::ComboBox^ cb_size;
	private: System::Windows::Forms::Timer^ timer1;
	private: System::Windows::Forms::TabControl^ tab_page;
	private: System::Windows::Forms::TabPage^ tp_1;
	private: System::Windows::Forms::TabPage^ tp_2;
	private: System::Windows::Forms::TabPage^ tp_3;
	private: System::Windows::Forms::TabPage^ tp_4;
	private: System::Windows::Forms::Button^ button2;
	private: System::Windows::Forms::SaveFileDialog^ saveFileDialog1;
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
			this->button1 = ( gcnew System::Windows::Forms::Button( ) );
			this->exp_list_1 = ( gcnew System::Windows::Forms::ListBox( ) );
			this->exp_list_2 = ( gcnew System::Windows::Forms::ListBox( ) );
			this->exp_list_3 = ( gcnew System::Windows::Forms::ListBox( ) );
			this->exp_list_4 = ( gcnew System::Windows::Forms::ListBox( ) );
			this->tb_address = ( gcnew System::Windows::Forms::TextBox( ) );
			this->cb_type = ( gcnew System::Windows::Forms::ComboBox( ) );
			this->cb_size = ( gcnew System::Windows::Forms::ComboBox( ) );
			this->timer1 = ( gcnew System::Windows::Forms::Timer( this->components ) );
			this->tab_page = ( gcnew System::Windows::Forms::TabControl( ) );
			this->tp_1 = ( gcnew System::Windows::Forms::TabPage( ) );
			this->tp_2 = ( gcnew System::Windows::Forms::TabPage( ) );
			this->tp_3 = ( gcnew System::Windows::Forms::TabPage( ) );
			this->tp_4 = ( gcnew System::Windows::Forms::TabPage( ) );
			this->button2 = ( gcnew System::Windows::Forms::Button( ) );
			this->saveFileDialog1 = ( gcnew System::Windows::Forms::SaveFileDialog( ) );
			this->tab_page->SuspendLayout( );
			this->tp_1->SuspendLayout( );
			this->tp_2->SuspendLayout( );
			this->tp_3->SuspendLayout( );
			this->tp_4->SuspendLayout( );
			this->SuspendLayout( );
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point( 382, 561 );
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size( 85, 27 );
			this->button1->TabIndex = 0;
			this->button1->Text = L"Add";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler( this, &form_main::Button1_Click );
			// 
			// exp_list_1
			// 
			this->exp_list_1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->exp_list_1->Font = ( gcnew System::Drawing::Font( L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
									   static_cast<System::Byte>( 0 ) ) );
			this->exp_list_1->FormattingEnabled = true;
			this->exp_list_1->ItemHeight = 17;
			this->exp_list_1->Location = System::Drawing::Point( 3, 3 );
			this->exp_list_1->Name = L"exp_list_1";
			this->exp_list_1->Size = System::Drawing::Size( 439, 508 );
			this->exp_list_1->TabIndex = 1;
			// 
			// exp_list_2
			// 
			this->exp_list_2->Dock = System::Windows::Forms::DockStyle::Fill;
			this->exp_list_2->FormattingEnabled = true;
			this->exp_list_2->ItemHeight = 15;
			this->exp_list_2->Location = System::Drawing::Point( 3, 3 );
			this->exp_list_2->Name = L"exp_list_2";
			this->exp_list_2->Size = System::Drawing::Size( 439, 508 );
			this->exp_list_2->TabIndex = 1;
			// 
			// exp_list_3
			// 
			this->exp_list_3->Dock = System::Windows::Forms::DockStyle::Fill;
			this->exp_list_3->FormattingEnabled = true;
			this->exp_list_3->ItemHeight = 15;
			this->exp_list_3->Location = System::Drawing::Point( 3, 3 );
			this->exp_list_3->Name = L"exp_list_3";
			this->exp_list_3->Size = System::Drawing::Size( 518, 511 );
			this->exp_list_3->TabIndex = 1;
			// 
			// exp_list_4
			// 
			this->exp_list_4->Dock = System::Windows::Forms::DockStyle::Fill;
			this->exp_list_4->FormattingEnabled = true;
			this->exp_list_4->ItemHeight = 15;
			this->exp_list_4->Location = System::Drawing::Point( 3, 3 );
			this->exp_list_4->Name = L"exp_list_4";
			this->exp_list_4->Size = System::Drawing::Size( 518, 511 );
			this->exp_list_4->TabIndex = 1;
			// 
			// tb_address
			// 
			this->tb_address->Location = System::Drawing::Point( 14, 564 );
			this->tb_address->Name = L"tb_address";
			this->tb_address->Size = System::Drawing::Size( 120, 23 );
			this->tb_address->TabIndex = 6;
			// 
			// cb_type
			// 
			this->cb_type->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cb_type->FormattingEnabled = true;
			this->cb_type->Items->AddRange( gcnew cli::array< System::Object^  >( 3 ) { L"Execute", L"Read/Write", L"Read" } );
			this->cb_type->Location = System::Drawing::Point( 142, 564 );
			this->cb_type->Name = L"cb_type";
			this->cb_type->Size = System::Drawing::Size( 114, 23 );
			this->cb_type->TabIndex = 7;
			// 
			// cb_size
			// 
			this->cb_size->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->cb_size->FormattingEnabled = true;
			this->cb_size->Items->AddRange( gcnew cli::array< System::Object^  >( 4 ) { L"1 Byte", L"2 Byte", L"8 Byte", L"4 Byte" } );
			this->cb_size->Location = System::Drawing::Point( 262, 564 );
			this->cb_size->Name = L"cb_size";
			this->cb_size->Size = System::Drawing::Size( 114, 23 );
			this->cb_size->TabIndex = 8;
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 2000;
			this->timer1->Tick += gcnew System::EventHandler( this, &form_main::Timer1_Tick );
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
			this->tp_1->Controls->Add( this->exp_list_1 );
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
			this->tp_2->Controls->Add( this->exp_list_2 );
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
			this->tp_3->Controls->Add( this->exp_list_3 );
			this->tp_3->Location = System::Drawing::Point( 4, 22 );
			this->tp_3->Name = L"tp_3";
			this->tp_3->Padding = System::Windows::Forms::Padding( 3 );
			this->tp_3->Size = System::Drawing::Size( 524, 517 );
			this->tp_3->TabIndex = 2;
			this->tp_3->Text = L"0";
			this->tp_3->UseVisualStyleBackColor = true;
			// 
			// tp_4
			// 
			this->tp_4->Controls->Add( this->exp_list_4 );
			this->tp_4->Location = System::Drawing::Point( 4, 22 );
			this->tp_4->Name = L"tp_4";
			this->tp_4->Padding = System::Windows::Forms::Padding( 3 );
			this->tp_4->Size = System::Drawing::Size( 524, 517 );
			this->tp_4->TabIndex = 3;
			this->tp_4->Text = L"0";
			this->tp_4->UseVisualStyleBackColor = true;
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point( 14, 594 );
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size( 453, 31 );
			this->button2->TabIndex = 10;
			this->button2->Text = L"Save logs";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler( this, &form_main::button2_Click );
			// 
			// form_main
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF( 7, 15 );
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size( 481, 636 );
			this->Controls->Add( this->button2 );
			this->Controls->Add( this->tab_page );
			this->Controls->Add( this->cb_size );
			this->Controls->Add( this->cb_type );
			this->Controls->Add( this->tb_address );
			this->Controls->Add( this->button1 );
			this->Font = ( gcnew System::Drawing::Font( L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
						   static_cast<System::Byte>( 0 ) ) );
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->Name = L"form_main";
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
	private: System::Void Button1_Click(System::Object^ sender, System::EventArgs^ e);
	private: System::Void Timer1_Tick(System::Object^ sender, System::EventArgs^ e);
	private: System::Void button2_Click( System::Object^ sender, System::EventArgs^ e );
};
}
