<div class="config_directives" style="display: none">
	<com:TActiveRepeater
		ID="RepeaterDirectives"
		OnItemDataBound="createDirectiveElement"
		>
		<prop:ItemTemplate>
			<div class="directive_field">
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'TextBox')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveText />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'Integer')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveInteger />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'CheckBox')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveBoolean />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'TimePeriod')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveTimePeriod />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'ComboBox')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveComboBox />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'FileSet')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveFileSet />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'Schedule')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveSchedule />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'Messages')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveMessages />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
				<com:Application.Web.Portlets.BConditional BCondition="<%#($this->DataItem['field_type'] === 'Runscript')%>">
					<prop:TrueTemplate>
						<com:Application.Web.Portlets.DirectiveRunscript />
					</prop:TrueTemplate>
				</com:Application.Web.Portlets.BConditional>
			</div>
		</prop:ItemTemplate>
	</com:TActiveRepeater>
	<com:Application.Web.Portlets.DirectiveSetting
		Resource="<%=$this->getResource()%>"
		OnLoadDirectives="loadDirectives"
	/>
	<div class="button center block">
		<com:BActiveButton
			ID="Save"
			ValidationGroup="Directive"
			ActiveControl.EnableUpdate="false"
			OnCommand="SourceTemplateControl.saveResource"
			CommandParameter="save"
			CssClass="bbutton"
			Text="<%[ Save ]%>"
		>
			<prop:ClientSide.OnLoading>
				$('.save_progress').show();
				$('.save_done').hide();
			</prop:ClientSide.OnLoading>
			<prop:ClientSide.OnComplete>
				$('.save_progress').hide();
				$('.save_done').show();
			</prop:ClientSide.OnComplete>
			<prop:ClientSide.OnFailure>
				$('.save_progress').hide();
				$('.save_done').show();
			</prop:ClientSide.OnFailure>
		</com:BActiveButton>
		<span style="display: inline-block; width: 60px;">
			<span class="save_progress" style="display: none;"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/ajax-loader.gif" alt="" /></span>
			<com:TActiveLabel ID="SaveDirectiveOk" Display="None" CssClass="txt-noshadow"><span class="save_done"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_ok.png" alt="" /> <%[ OK ]%></span></com:TActiveLabel>
			<com:TActiveLabel ID="SaveDirectiveError" Display="None" CssClass="txt-noshadow error"><span class="save_done"><img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/icon_err.png" alt="" /> <%[ Error ]%></span></com:TActiveLabel>
		</span>
		<com:TActiveLabel ID="SaveDirectiveErrMsg" Display="None" CssClass="txt-noshadow error block" />
	</div>
</div>
