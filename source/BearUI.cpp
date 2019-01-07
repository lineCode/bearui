#include "BearUI.hpp"
#ifdef WINDOWS
BearCore::BearMap<int32, int32>* GWindowsKeys;
#endif


BearUI::BearUI::BearUI(bsize width, bsize height):m_focus_item(0)
{
	BearGraphics::BearShaderConstantsInitializer sconst_othro;
	sconst_othro.Constants.push_back(BearGraphics::CF_MATRIX);
	m_matrix_constant = BearGraphics::BearShaderConstantsRef(sconst_othro, true);

	m_matrix.BuildOrthoOffCenter(static_cast<float>(width), static_cast<float>(height), FLT_MIN, 100.f);
	BearCore::bear_copy(m_matrix_constant.Lock(), *m_matrix, sizeof(float) * 16);
	m_matrix_constant.Unlock();

	BearGraphics::BearShaderConstantsInitializer sconst_color;
	sconst_color.Constants.push_back(BearGraphics::CF_R32G32B32A32_FLOAT);
	m_color_constant = BearGraphics::BearShaderConstantsRef(sconst_color, true);



	m_vertex_buffer.Create(sizeof(BearGraphics::BearVertexDefault) * 6);
	static int32 index_buffer[] = { 2,1,0,1,3,0 };
	m_index_buffer.Create(index_buffer, sizeof(index_buffer), false);

	m_size_screen.set(width, height);
}
BearUI::BearUI::~BearUI()
{
	m_color_constant.Clear();
	m_matrix_constant.Clear();
	m_vertex_buffer.Clear();
	m_index_buffer.Clear();
#ifdef WINDOWS
	BearCore::bear_free(GWindowsKeys);
#endif
}

BearUI::BearFontRef BearUI::BearUI::GetFont(FontLang lang, bsize size)
{
	const bchar16*chars = L"";

	switch (lang)
	{
	case BearUI::BearUI::F_ENG:
		chars = L"0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM.,?/\\*-_+=(){}[]&^:;%$#�@\"'!`~><";
		break;
	case BearUI::BearUI::F_RUS:
		chars = L"0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM���������������������������������������������������������������.,?/\\*-_+=(){}[]&^:;%$#�@\"'!`~��><";
		break;
	default:
		break;
	}

	auto item = m_fonts.find(FontInfo(lang, size));
	if (item == m_fonts.end())
	{
		m_fonts.insert(FontInfo(lang, size));
		item = m_fonts.find(FontInfo(lang, size));
		item->second.LoadFromTTF(TEXT("C:/Windows/Fonts/arial.ttf"), chars, m_size_screen.x, m_size_screen.y, size);
	}
	return 	item->second;
}

BearUI::BearUIItem * BearUI::BearUI::PushItem(BearUIItem * item)
{
	item->Reset();
	m_items.push_back(item);
	return item;
}

BearUI::BearUIStaticItem * BearUI::BearUI::PushItem(BearUIStaticItem * item)
{
	item->Reset();
	m_static_items.push_back(item);
	return item;
}

void BearUI::BearUI::Resize(bsize width, bsize height)
{
	m_matrix.BuildOrthoOffCenter(static_cast<float>(width), static_cast<float>(height), FLT_MIN, 100.f);
	BearCore::bear_copy(m_matrix_constant.Lock(), *m_matrix, sizeof(float) * 16);
	m_matrix_constant.Unlock();
	m_size_screen.set(width, height);
	Reset();
}


void BearUI::BearUI::Draw(float time)
{
	BearGraphics::BearRenderInterface::SetBlendState(BearGraphics::BearDefaultManager::GetBlendAlpha(), BearCore::BearColor::Transparent);
	BearGraphics::BearRenderInterface::SetRasterizerState(BearGraphics::BearDefaultManager::GetRasterizerState());
	BearGraphics::BearRenderInterface::SetVertexState(BearGraphics::BearDefaultManager::GetVertexState());
	{
		auto b = m_items.rbegin();
		auto e = m_items.rend();
		while (b != e)
		{
			(*b)->Draw(this, time);
			b++;
		}
	}
	{
		auto b = m_static_items.rbegin();
		auto e = m_static_items.rend();
		while (b != e)
		{
			(*b)->Draw(this, time);
			b++;
		}
	}
}

void BearUI::BearUI::Update(float time)
{
	auto b = m_items.rbegin();
	auto e = m_items.rend();
	while (b != e)
	{
		(*b)->Update();
		b++;
	}
	
}

void BearUI::BearUI::Unload()
{
	auto b = m_items.begin();
	auto e = m_items.end();
	while (b != e)
	{
		(*b)->Unload();
		b++;
	}
}

void BearUI::BearUI::Reload()
{
	auto b = m_items.begin();
	auto e = m_items.end();
	while (b != e)
	{
		(*b)->Reload();
		b++;
	}
}

void BearUI::BearUI::Reset()
{
	auto b = m_items.begin();
	auto e = m_items.end();
	while (b != e)
	{
		(*b)->Reset();
		b++;
	}
}

void BearUI::BearUI::KillFocus()
{
	m_focus_item = 0;
	auto b = m_items.begin();
	auto e = m_items.end();
	while (b != e)
	{
		(*b)->KillFocus();
		b++;
	}
}

void BearUI::BearUI::OnMouse(float x, float y)
{
	if (m_focus_item&&m_focus_item->OnMouse(x, y))
		return;
	auto b = m_items.begin();
	auto e = m_items.end();
	while (b != e)
	{
		if ((*b) != m_focus_item)
			if ((*b)->OnMouse(x, y))
				break;
		b++;
	}
}

void BearUI::BearUI::OnKeyDown(BearInput::Key key)
{
	if (m_focus_item&&m_focus_item->OnKeyDown(key))
		return;

	auto b = m_items.begin();
	auto e = m_items.end();
	while (b != e)
	{
		if (m_focus_item !=*b&&(*b)->OnKeyDown(key))
		{
			m_focus_item = (*b);
			UpdateFocus();
			break;
		}
		b++;
	}
}

void BearUI::BearUI::OnKeyUp(BearInput::Key key)
{
	if (m_focus_item&&m_focus_item->OnKeyUp(key))
		return;
	auto b = m_items.begin();
	auto e = m_items.end();
	while (b != e)
	{
		if (m_focus_item != *b &&(*b)->OnKeyUp(key))break;
		b++;
	}
}


void BearUI::BearUI::Render(BearUITexture * texture)
{
    BearGraphics::BearRenderInterface::SetPixelShader(BearGraphics::BearDefaultManager::GetPixelShader(BearGraphics::DPS_UITexture));
	BearGraphics::BearRenderInterface::SetVertexShader(BearGraphics::BearDefaultManager::GetVertexShader(BearGraphics::DVS_Default));

	BearGraphics::BearRenderInterface::SetVertexShaderConstants(0, m_matrix_constant);

	if (texture->Flags.test(BearUIStaticItem::UI_NoClip))
	{
		BearGraphics::BearRenderInterface::SetScissor( 0, 0, static_cast<float>(m_size_screen.x), static_cast<float>(m_size_screen.y));
	}
	else
	{
		BearGraphics::BearRenderInterface::SetScissor( texture->Clip.x, texture->Clip.y, texture->Clip.x+ texture->Clip.x1, texture->Clip.y+ texture->Clip.y1);
	}

	BearCore::bear_copy(reinterpret_cast<float*>(m_color_constant.Lock()), texture->Color.GetFloat().array, 4);
	m_color_constant.Unlock();
	BearGraphics::BearRenderInterface::SetPixelShaderConstants(0, m_color_constant);

	BearGraphics::BearRenderInterface::SetPixelShaderResource(0, texture->Texture, BearGraphics::BearDefaultManager::GetSamplerState());
	BearCore::bear_copy(reinterpret_cast<BearGraphics::BearVertexDefault*>(m_vertex_buffer.Lock()), texture->m_vertex, 4);
	m_vertex_buffer.Unlock();
	BearGraphics::BearRenderInterface::SetVertexBuffer(m_vertex_buffer);
	BearGraphics::BearRenderInterface::SetIndexBuffer(m_index_buffer);
	BearGraphics::BearRenderInterface::Draw(6);
}

void BearUI::BearUI::Render(BearUIText * text)
{
	BearGraphics::BearVertexDefault vertex[4];

	bsize size = text->Text.size();
	if (text->Font.Empty() || !size)return;

	BearGraphics::BearRenderInterface::SetPixelShader(BearGraphics::BearDefaultManager::GetPixelShader(BearGraphics::DPS_UIText));
	BearGraphics::BearRenderInterface::SetVertexShader(BearGraphics::BearDefaultManager::GetVertexShader(BearGraphics::DVS_Default));

	BearGraphics::BearRenderInterface::SetVertexShaderConstants(0, m_matrix_constant);

	if (text->Flags.test(BearUIStaticItem::UI_NoClip))
	{
		BearGraphics::BearRenderInterface::SetScissor(0, 0, static_cast<float>(m_size_screen.x), static_cast<float>(m_size_screen.y));
	}
	else
	{
		BearGraphics::BearRenderInterface::SetScissor(text->Clip.x, text->Clip.y, text->Clip.x + text->Clip.x1, text->Clip.y + text->Clip.y1);
	}

	BearCore::bear_copy(reinterpret_cast<float*>(m_color_constant.Lock()), text->Color.GetFloat().array, 4);
	m_color_constant.Unlock();

	BearGraphics::BearRenderInterface::SetPixelShaderConstants(0, m_color_constant);
	BearGraphics::BearRenderInterface::SetPixelShaderResource(0, *text->Font.GetTexture(), BearGraphics::BearDefaultManager::GetSamplerState());
	BearGraphics::BearRenderInterface::SetVertexBuffer(m_vertex_buffer);
	BearGraphics::BearRenderInterface::SetIndexBuffer(m_index_buffer);

	auto&font = *text->Font.GetListChars();
	auto pos = text->Position+text->ShiftPosition;
	BearCore::BearVector4<float> TextureUV;

	for (bsize i = 0; i < size; i++)
	{
		bchar16 ch_ = BearCore::BearEncoding::ToUTF16(text->Text[i]);
		if (ch_ == TEXT(' '))
		{
			auto item = font.find(ch_);
			pos.x += item->second.Advance;
		}
		else if (ch_ == TEXT('\t'))
		{
			ch_ = TEXT(' ');
			auto item = font.find(ch_);
			pos.x += item->second.Advance * 4;
		}
		else if (ch_ == TEXT('\n'))
		{
			pos.y += text->Font.GetHieght();
			pos.x = text->Position.x+ text->ShiftPosition.x;
		}
		else if (ch_ != TEXT('\r'))
		{
			auto item = font.find(ch_);
			if (item != font.end())
			{

				TextureUV = item->second.TextureUV;
				if (pos.x + item->second.Advance > text->Rect.x1 + text->Position.x&&text->Style.test(text->ST_MaxWidth))
				{
					pos.y += text->Font.GetHieght();
					pos.x = text->Position.x;
				}

				{
					float x = pos.x + item->second.Position.x, y = pos.y + item->second.Position.y;
					float width = item->second.Size.x;
					float height = item->second.Size.y;

					vertex[0].Position.set(x, y + height,0);
					vertex[1].Position.set(x + width, y,0);
					vertex[2].Position.set(x, y,0);
					vertex[3].Position.set(x + width, y + height,0);
					vertex[0].TextureUV.set(TextureUV.x, TextureUV.y + TextureUV.y1);
					vertex[1].TextureUV.set(TextureUV.x1 + TextureUV.x, TextureUV.y);
					vertex[2].TextureUV.set(TextureUV.x, TextureUV.y);
					vertex[3].TextureUV.set(TextureUV.x1 + TextureUV.x, TextureUV.y1 + TextureUV.y);

					BearCore::bear_copy(reinterpret_cast<BearGraphics::BearVertexDefault*>(m_vertex_buffer.Lock()), vertex, 4);
					m_vertex_buffer.Unlock();
				
					BearGraphics::BearRenderInterface::Draw(6);

				}
				pos.x += item->second.Advance;
			}
		}

	}
}

void BearUI::BearUI::UpdateFocus()
{
	auto b = m_items.begin();
	auto e = m_items.end();
	while (b != e)
	{
		if (*b != m_focus_item)(*b)->KillFocus();
		b++;
	}
}

