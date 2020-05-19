<!-- Sidebar/menu -->
<nav class="w3-sidebar w3-white w3-animate-left w3-margin-bottom" style="z-index:3;width:300px;" id="sidebar"><br />
	<div class="w3-container w3-row">
		<div class="w3-col s4">
			<img src="<%=$this->getPage()->getTheme()->getBaseUrl()%>/avatar2.png" class="w3-circle w3-margin-right" style="width:46px" />
		</div>
		<div class="w3-col s8 w3-bar">
			<span><%[ Welcome ]%>, <strong><%=$this->User->getUsername()%></strong></span><br>
			<script>var main_side_bar_reload_url = '<%=$this->reload_url%>';</script>
			<com:TActiveLinkButton
				ID="Logout"
				OnClick="logout"
				CssClass="w3-bar-item w3-button"
				ToolTip="<%[ Logout ]%>"
			>
				<prop:ClientSide.OnComplete>
					if (!window.chrome || window.navigator.webdriver)  {
						window.location.href = main_side_bar_reload_url;
					} else if (window.chrome) {
						// For chrome this reload is required to show login Basic auth prompt
						window.location.reload();
					}
				</prop:ClientSide.OnComplete>
				<i class="fa fa-power-off"></i>
			</com:TActiveLinkButton>
			<a href="<%=$this->Service->constructUrl('Console')%>" class="w3-bar-item w3-button<%=$this->getModule('users')->isPageAllowed($this->User, 'Console') ? '' : ' hide'%>" title="<%[ Console ]%>"><i class="fa fa-terminal"></i></a>
			<a href="<%=$this->Service->constructUrl('ApplicationSettings')%>" class="w3-bar-item w3-button<%=$this->getModule('users')->isPageAllowed($this->User, 'ApplicationSettings') ? '' : ' hide'%>" title="<%[ Application settings ]%>"><i class="fa fa-cog"></i></a>
		</div>
	</div>
	<hr />
	<div class="w3-container w3-black">
		<h5>Baculum Menu</h5>
	</div>
	<div class="w3-bar-block" style="margin-bottom: 45px;">
		<!--a href="#" class="w3-bar-item w3-button w3-padding-16 w3-black w3-hover-black w3-hide-large" onclick="W3SideBar.close(); return false;" title="close menu">  <%[ Close Menu ]%> <i class="fa fa-window-close fa-fw w3-right w3-xlarge"></i></a-->
		<div class="w3-black" style="height: 3px"></div>
		<a href="<%=$this->Service->constructUrl('Dashboard')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'Dashboard' ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'Dashboard') ? '' : ' hide'%>"><i class="fa fa-tachometer-alt fa-fw"></i>  <%[ Dashboard ]%></a>
		<a href="<%=$this->Service->constructUrl('JobHistoryList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('JobHistoryList', 'JobHistoryView')) ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'JobHistoryList') ? '' : ' hide'%>"><i class="fa fa-history fa-fw"></i>  <%[ Job history ]%></a>
		<a href="<%=$this->Service->constructUrl('JobList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('JobList', 'JobView')) ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'JobList') ? '' : ' hide'%>"><i class="fa fa-tasks fa-fw"></i>  <%[ Jobs ]%></a>
		<a href="<%=$this->Service->constructUrl('ClientList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('ClientList', 'ClientView')) ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'ClientList') ? '' : ' hide'%>"><i class="fa fa-desktop fa-fw"></i>  <%[ Clients ]%></a>
		<a href="<%=$this->Service->constructUrl('StorageList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('StorageList', 'StorageView', 'DeviceView')) ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'StorageList') ? '' : ' hide'%>"><i class="fa fa-database fa-fw"></i>  <%[ Storages ]%></a>
		<a href="<%=$this->Service->constructUrl('PoolList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('PoolList', 'PoolView')) ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'PoolList') ? '' : ' hide'%>"><i class="fa fa-tape fa-fw"></i>  <%[ Pools ]%></a>
		<a href="<%=$this->Service->constructUrl('VolumeList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('VolumeList', 'VolumeView')) ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'VolumeList') ? '' : ' hide'%>"><i class="fa fa-hdd fa-fw"></i>  <%[ Volumes ]%></a>
		<a href="<%=$this->Service->constructUrl('FileSetList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('FileSetList', 'FileSetView')) ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'FileSetList') ? '' : ' hide'%>"><i class="fa fa-copy fa-fw"></i>  <%[ FileSets ]%></a>
		<a href="<%=$this->Service->constructUrl('ScheduleList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('ScheduleList', 'ScheduleView')) ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'ScheduleList') ? '' : ' hide'%>"><i class="fa fa-clock fa-fw"></i>  <%[ Schedules ]%></a>
		<a href="<%=$this->Service->constructUrl('ConfigureHosts')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'ConfigureHosts' ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'ConfigureHosts') ? '' : ' hide'%>"><i class="fa fa-cog fa-fw"></i>  <%[ Configure ]%></a>
		<a href="<%=$this->Service->constructUrl('RestoreWizard')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'RestoreWizard' ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'RestoreWizard') ? '' : ' hide'%>"><i class="fa fa-reply fa-fw"></i>  <%[ Restore wizard ]%></a>
		<a href="<%=$this->Service->constructUrl('Graphs')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'Graphs' ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'Graphs') ? '' : ' hide'%>"><i class="fa fa-chart-pie fa-fw"></i>  <%[ Graphs ]%></a>
		<a href="<%=$this->Service->constructUrl('StatisticsList')%>" class="w3-bar-item w3-button w3-padding<%=in_array($this->Service->getRequestedPagePath(), array('StatisticsList', 'StatisticsView')) ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'StatisticsList') ? '' : ' hide'%>"><i class="fas fa-chart-line fa-fw"></i>  <%[ Statistics ]%></a>
		<a href="<%=$this->Service->constructUrl('WebConfigWizard')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'WebConfigWizard' ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'WebConfigWizard') ? '' : ' hide'%>"><i class="fa fa-wrench fa-fw"></i>  <%[ Settings ]%></a>
		<a href="<%=$this->Service->constructUrl('Security')%>" class="w3-bar-item w3-button w3-padding<%=$this->Service->getRequestedPagePath() == 'Security' ? ' w3-blue': ''%><%=$this->getModule('users')->isPageAllowed($this->User, 'Security') ? '' : ' hide'%>"><i class="fa fa-lock fa-fw"></i>  <%[ Security ]%></a>
	</div>
</nav>

<!-- Overlay effect when opening sidebar on small screens -->
<div class="w3-overlay w3-hide-large w3-animate-opacity" onclick="W3SideBar.close(); return false;" style="cursor:pointer" title="close side menu" id="overlay_bg"></div>
